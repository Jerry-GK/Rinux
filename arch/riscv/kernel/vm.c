#include "defs.h"
#include "types.h"
#include "printk.h"
#include "string.h"
#include "mm.h"
#include "vm.h"

extern char _stext[];
extern char _etext[];
extern char _srodata[];
extern char _erodata[];
extern char _sdata[];
extern char _edata[];

#define VPN0MASK ((uint64) 0x1FF << 12)
#define VPN1MASK ((uint64) 0x1FF << 21)
#define VPN2MASK ((uint64) 0x1FF << 30)

#define PPN0MASK ((uint64) 0x1FF << 12)
#define PPN1MASK ((uint64) 0x1FF << 21)
#define PPN2MASK ((uint64) 0x3FFFFFF << 30)

#define PTEMASK (((uint64) 0xFFFFFFFFFFF) << 10)

/*
 63      54 53        28 27        19 18        10 9   8 7 6 5 4 3 2 1 0
 -----------------------------------------------------------------------
| Reserved |   PPN[2]   |   PPN[1]   |   PPN[0]   | RSW |D|A|G|U|X|W|R|V|
 -----------------------------------------------------------------------
                    PTE

38        30 29        21 20        12 11                           0
---------------------------------------------------------------------
|   VPN[2]   |   VPN[1]   |   VPN[0]   |          page offset         |
---------------------------------------------------------------------
                    Sv39 virtual address

 55                30 29        21 20        12 11                           0
 -----------------------------------------------------------------------------
|       PPN[2]       |   PPN[1]   |   PPN[0]   |          page offset         |
 -----------------------------------------------------------------------------
                    Sv39 physical address
*/


/* pgtbl_prepare: 用于 setup_vm_prepare 进行 1GB 的 映射。 */
unsigned long  pgtbl_prepare[512] __attribute__((__aligned__(0x1000)));

/* swapper_pg_dir: kernel pagetable 根目录， 在 setup_vm_map 进行映射。 */
unsigned long  swapper_pg_dir[512] __attribute__((__aligned__(0x1000)));

void setup_vm_prepare(void) {
    /* 
    1. 由于是进行 1GB 的映射 这里不需要使用多级页表 
    2. 将 va 的 64bit 作为如下划分： | high bit | 9 bit | 30 bit |
        high bit 可以忽略
        中间9 bit 作为 pgtbl_prepare 的 index
        低 30 bit 作为 页内偏移 这里注意到 30 = 9 + 9 + 12， 即我们只使用根页表， 根页表的每个 entry 都对应 1GB 的区域。 
    3. Page Table Entry 的权限 V | R | W | X 位设置为 1
    */

    /*
    注意此函数将虚拟内存中0x80000000 - 0x81000000和0xffffffe000000000 - 0xffffffe001000000这两段1GB的内容都映射到了
    物理内存的0x80000000 - 0x81000000位置

    Physical Address
    -------------------------------------------
                        | OpenSBI | Kernel |
    -------------------------------------------
                        ^
                    0x80000000 - 0x81000000(1GB)
                        ├───────────────────────────────────────────────────┐
                        |                                                   |
    Virtual Address      ↓                                                   ↓
    -----------------------------------------------------------------------------------------------
                        | OpenSBI | Kernel |                                | OpenSBI | Kernel |
    -----------------------------------------------------------------------------------------------
                        ^                                                   ^
                    0x80000000 - 0x81000000(1GB)                    0xffffffe000000000 - 0xffffffe001000000(1GB)
    */
    memset(pgtbl_prepare, 0, PGSIZE);
    
    uint64 addr_phy_start = PHY_START;
    uint64 phy_index = (addr_phy_start & PPN2MASK) >> 30; //phy_index = PPN2 of addr_phy_start
    pgtbl_prepare[phy_index] = (phy_index << 28) | 0xF; //set PPN[2]. DAGUXWRV = 00001111。后几位不全为0，一级页表直接映射

    uint64 addr_vm_start= VM_START;
    uint64 vm_index = (addr_vm_start & VPN2MASK) >> 30;
    pgtbl_prepare[vm_index] = (phy_index << 28) | 0xF; //映射到同一段物理内存
}

void setup_vm_map(void) {
    memset(swapper_pg_dir, 0x0, PGSIZE);    
    uint64 vm_addr_start = 0;
    uint64 vm_addr_end = 0;
    // No OpenSBI mapping required

    // mapping kernel text X|-|R|V
    vm_addr_start = (uint64)_stext;
    vm_addr_end = (uint64)_etext;
    create_mapping(swapper_pg_dir, vm_addr_start, vm_addr_start - VA_PA_OFFSET, vm_addr_end - vm_addr_start, 0x5);
    //test_all_addr(vm_addr_start, vm_addr_end);

    // mapping kernel rodata -|-|R|V
    vm_addr_start = (uint64)_srodata;
    vm_addr_end = (uint64)_erodata;
    create_mapping(swapper_pg_dir, vm_addr_start, vm_addr_start - VA_PA_OFFSET, vm_addr_end - vm_addr_start, 0x1);
    //test_all_addr(vm_addr_start, vm_addr_end);

    // mapping other memory -|W|R|V
    vm_addr_start = (uint64)_sdata;
    vm_addr_end = (uint64)(VM_START + PHY_SIZE);//注意这里不是_edata
    create_mapping(swapper_pg_dir, vm_addr_start, vm_addr_start - VA_PA_OFFSET, vm_addr_end - vm_addr_start, 0x3);
    //test_all_addr(vm_addr_start, vm_addr_end);

    // set satp with swapper_pg_dir
    uint64 swapper_pg_dir_phy_addr = (uint64)(swapper_pg_dir) - VA_PA_OFFSET;
    printk("pd_pa = %lx\n", swapper_pg_dir_phy_addr);
    csr_write(satp, ((swapper_pg_dir_phy_addr>>12) | ((uint64)0x8 << 60)));

    //test_all_addr(vm_addr_start, vm_addr_end);

    // flush TLB
    asm volatile("sfence.vma zero, zero");
    
    printk("[Initialize] Virtual memory mapping done!\n");
    return;
}


/* 创建多级(3级)页表映射关系 */
void create_mapping(uint64 *pgtbl, uint64 va, uint64 pa, uint64 sz, int perm) {
    /*
    pgtbl 为根页表的基地址
    va, pa 为需要映射的虚拟地址、物理地址
    sz 为映射的大小
    perm 为映射的读写权限

    创建多级页表的时候可以使用 kalloc() 来获取一页作为页表目录
    可以使用 V bit 来判断页表项是否存在
    */

    //printk("Mapping: va=%lx pa=%lx sz=%d perm=%d\n", va, pa, sz, perm);
    uint64* pgtbl1, *pgtbl0;
    uint64 paddr, vaddr, va_idx2, va_idx1,va_idx0;
    for(uint64 offset = 0; offset < sz; offset += PGSIZE)
    {
        paddr = pa + offset;
        vaddr = va + offset;
        va_idx2 = (vaddr & VPN2MASK) >> 30;
        va_idx1 = (vaddr & VPN1MASK) >> 21;
        va_idx0 = (vaddr & VPN0MASK) >> 12;

        if(getPTEBit(pgtbl[va_idx2], 0))
        {
            pgtbl1 = (uint64*)(((((uint64)pgtbl[va_idx2] & PTEMASK) >> 10) << 12) + VA_PA_OFFSET);
            if(getPTEBit(pgtbl1[va_idx1], 0))
            {
                pgtbl0 = (uint64*)(((((uint64)pgtbl1[va_idx1] & PTEMASK) >> 10) << 12) + VA_PA_OFFSET);
                if(getPTEBit(pgtbl0[va_idx0], 0))
                {
                    //覆盖设置物理地址 or 不覆盖？
                    continue;//不覆盖
                }
                else//0级页表缺项，直接存放物理页号
                {
                    pgtbl0[va_idx0] = ((paddr >> 12) << 10) | ((perm << 1) | 0x1);
                }
            }
            else//1级页表缺项，申请0级页表
            {
                pgtbl0 = (uint64*)kalloc();
                pgtbl1[va_idx1] = ((((uint64)pgtbl0 - VA_PA_OFFSET) >> 12) << 10) | 0x1;
                pgtbl0[va_idx0] = ((paddr >> 12) << 10) | ((perm << 1) | 0x1);
            }
        }
        else//2级页表缺项，申请1级和0级页表
        {
            pgtbl1 = (uint64*)kalloc();
            pgtbl0 = (uint64*)kalloc();

            pgtbl[va_idx2] = ((((uint64)pgtbl1 - VA_PA_OFFSET) >> 12) << 10) | 0x1;
            pgtbl1[va_idx1] = ((((uint64)pgtbl0 - VA_PA_OFFSET) >> 12) << 10) | 0x1;
            pgtbl0[va_idx0] = ((paddr >> 12) << 10) | ((perm << 1) | 0x1);
        }
    }
}

int getPTEBit(uint64 entry, unsigned int index)
{
    if(index>7)
        return 0;
    return (entry >> index) & 0x1;
}

void test_all_addr(uint64 va_begin, uint64 va_end)
{
    printk("begin address test, va: %lx -> %lx\n", va_begin, va_end);
    uint64 gap = 1;
    for(uint64 i = 0; va_begin + i < va_end; i+=gap)
    {
        uint64 va = va_begin + i;
        uint64 pa = get_pa(va);
        if(pa != va - VA_PA_OFFSET)
        {
            printk("Wrong match for va = %lx  pa = %lx pa_exp = %lx\n", va, pa, va - VA_PA_OFFSET);
        }
        else
        {
            //printk("Correct match for va = %lx  pa = %lx pa_exp = %lx\n", va, pa, va - VA_PA_OFFSET);
        }
    }
    printk("test address end!\n");
}

uint64 get_pa(uint64 va){
    uint64 * fir_pgtbl, * zero_pgtbl;
    uint64 sec_idx, fir_idx, zero_idx;
    sec_idx = (va & VPN2MASK) >> 30;
    fir_idx = (va & VPN1MASK) >> 21;
    zero_idx = (va & VPN0MASK) >> 12;
    fir_pgtbl = (uint64*) ((((uint64)swapper_pg_dir[sec_idx] & PTEMASK) >> 10 << 12) + VA_PA_OFFSET);
    zero_pgtbl = (uint64*) ((((uint64)fir_pgtbl[fir_idx] & PTEMASK) >> 10 << 12) + VA_PA_OFFSET);
    return (uint64)(((uint64)zero_pgtbl[zero_idx] >> 10 << 12) | ((uint64)va & 0xFFF));
}
