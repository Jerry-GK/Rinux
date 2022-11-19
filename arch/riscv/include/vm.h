void setup_vm_prepare(void);

void setup_vm_map(void);


/* 创建多级(3级)页表映射关系 */
void create_mapping(uint64 *pgtbl, uint64 va, uint64 pa, uint64 sz, int perm);

int getPTEBit(uint64 entry, unsigned int index);

void test_all_addr(uint64 va_begin, uint64 va_end);

uint64 get_pa(uint64 va);