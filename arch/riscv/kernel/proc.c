//arch/riscv/kernel/proc.c
#include "proc.h"
#include "defs.h"

extern void __dummy();

struct task_struct* idle;           // idle process
struct task_struct* current;        // ָ��ǰ�����̵߳� `task_struct`
struct task_struct* task[NR_TASKS]; // �߳����飬���е��̶߳������ڴ�

void task_init() {
    // 1. ���� kalloc() Ϊ idle ����һ������ҳ
    idle = (struct task_struct*)kalloc();
    // 2. ���� state Ϊ TASK_RUNNING;
    idle->state = TASK_RUNNING;
    // 3. ���� idle ��������� ���Խ��� counter / priority ����Ϊ 0
    idle->counter = 0;
    idle->priority = 0;
    // 4. ���� idle �� pid Ϊ 0
    idle->pid = 0;
    // 5. �� current �� task[0] ָ�� idle
    current = idle;
    task[0] = idle;

    /* YOUR CODE HERE */
    // 1. �ο� idle ������, Ϊ task[1] ~ task[NR_TASKS - 1] ���г�ʼ��
    // 2. ����ÿ���̵߳� state Ϊ TASK_RUNNING, counter Ϊ 0, priority ʹ�� rand() ������, pid Ϊ���߳����߳������е��±ꡣ
    // 3. Ϊ task[1] ~ task[NR_TASKS - 1] ���� `thread_struct` �е� `ra` �� `sp`,
    // 4. ���� `ra` ����Ϊ __dummy ���� 4.3.2���ĵ�ַ�� `sp` ����Ϊ ���߳����������ҳ�ĸߵ�ַ
    /* YOUR CODE HERE */
    for(uint64 i = 1; i < NR_TASKS; i++)
    {
        task[i] = (struct task_struct*)kalloc();
        task[i]->counter = 0;
        task[i]->priority = rand();
        task[i]->pid = i;

        task[i]->thread.ra = (uint64)&__dummy;
        task[i]->thread.sp = (uint64)task[i] + PGSIZE;
        //printk("task[%d]: add = %x, ra = %x, sp = %x\n", i, task[i], task[i]->thread.ra, task[i]->thread.sp);
    }

    printk("[Initialize] Process initialization done!\n");
}

extern void __switch_to(struct task_struct* prev, struct task_struct* next);

void switch_to(struct task_struct* next) {
    /* YOUR CODE HERE */
    if(current->pid == next->pid)
        return;
    struct task_struct* prev = current;
    current = next;
    __switch_to(prev, next);   
}

void do_timer(void) {
    /* 1. �����ǰ�߳���idle�̣߳�ֱ�ӽ��е���
    /* 2. �����ǰ�̲߳���idle���Ե�ǰ�̵߳�����ʣ��ʱ���1����ʣ��ʱ����Ȼ����0����ֱ�ӷ��أ�������е���*/

    /* YOUR CODE HERE */
    if(current == idle)
    {
        schedule();
    }
    else
    {
        current->counter -= 1;
        if(current->counter <= 0) //non-preemitive
            schedule();
        else
            return;
    }
}

void reset_all_counter()
{
    printk("[Resetting] all task counters:\n");
    for(uint64 i = 1; i < NR_TASKS; i++)
    {
        task[i]->counter = rand();
        printk("[SET] [PID = %d COUNTER = %d]\n", task[i]->pid, task[i]->counter);
    }
}

void reset_all_priority_counter()
{
    printk("[Resetting] all task priorities and counters:\n");
    for(int i = 1; i < NR_TASKS; i++)
    {
        task[i]->priority = rand();
        task[i] -> counter = (task[i] -> counter >> 1) + task[i] -> priority;
        printk("SET [PID = %d PRIORITY = %d COUNTER = %d]\n", task[i] -> pid, task[i] -> priority,task[i] -> counter);
    }
}

#ifdef SJF
void schedule(void) {
    /* YOUR CODE HERE */
    struct task_struct* next = task[0];

    while(1)
    {
        for(uint64 i = 1; i < NR_TASKS; i++)
        {
            if(task[i]->state == TASK_RUNNING && task[i]->counter > 0 
                && (task[i]->counter < next->counter || next->pid == 0))
            {
                next = task[i];
            }
        }

        if(next->pid != 0) //ѡ����һ������
            break;
        reset_all_counter();//����0���������н���counter��Ϊ0, ����counter���µ���
    }
    printk("[SWITCH] to [PID = %d COUNTER = %d] (SCHEME = SJF)\n", next->pid, next->counter);
    switch_to(next);
}
#elif PRIORITY
void schedule()
{
    /* YOUR CODE HERE */
    struct task_struct* next = task[0];

    while (1) 
    {
		int max_counter = 0;
        for(int i = 1; i < NR_TASKS; i++)
        {
            if(task[i] -> state == TASK_RUNNING && task[i] -> counter > max_counter)
            {
                next = task[i];
                max_counter = task[i] -> counter;
            }
        }
        //here, next has max counter, max_counter is the max counter value

		if(max_counter != 0)
            break;
        reset_all_priority_counter();
	}
    printk("[SWITCH] to [PID = %d COUNTER = %d] (SCHEME = PRIORITY)\n", next->pid, next->counter);
    switch_to(next);
}
#endif

void dummy() {
    uint64 MOD = 1000000007;
    uint64 auto_inc_local_var = 0;
    int last_counter = -1;
    while(1) {
        if (last_counter == -1 || current->counter != last_counter) {
            last_counter = current->counter;
            auto_inc_local_var = (auto_inc_local_var + 1) % MOD;
            printk("[PID = %d] is running. COUNTER = %d auto_inc_local_var = %d\n", 
                    current->pid, current->counter,auto_inc_local_var);
        }
    }
}



