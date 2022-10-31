//arch/riscv/kernel/proc.c
#include "proc.h"
#include "defs.h"

extern void __dummy();

struct task_struct* idle;           // idle process
struct task_struct* current;        // 指向当前运行线程的 `task_struct`
struct task_struct* task[NR_TASKS]; // 线程数组，所有的线程都保存在此

void task_init() {
    // 1. 调用 kalloc() 为 idle 分配一个物理页
    idle = (struct task_struct*)kalloc();
    // 2. 设置 state 为 TASK_RUNNING;
    idle->state = TASK_RUNNING;
    // 3. 由于 idle 不参与调度 可以将其 counter / priority 设置为 0
    idle->counter = 0;
    idle->priority = 0;
    // 4. 设置 idle 的 pid 为 0
    idle->pid = 0;
    // 5. 将 current 和 task[0] 指向 idle
    current = idle;
    task[0] = idle;

    /* YOUR CODE HERE */
    // 1. 参考 idle 的设置, 为 task[1] ~ task[NR_TASKS - 1] 进行初始化
    // 2. 其中每个线程的 state 为 TASK_RUNNING, counter 为 0, priority 使用 rand() 来设置, pid 为该线程在线程数组中的下标。
    // 3. 为 task[1] ~ task[NR_TASKS - 1] 设置 `thread_struct` 中的 `ra` 和 `sp`,
    // 4. 其中 `ra` 设置为 __dummy （见 4.3.2）的地址， `sp` 设置为 该线程申请的物理页的高地址
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
    /* 1. 如果当前线程是idle线程，直接进行调度
    /* 2. 如果当前线程不是idle，对当前线程的运行剩余时间减1；若剩余时间仍然大于0，则直接返回，否则进行调度*/

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

        if(next->pid != 0) //选中下一个进程
            break;
        reset_all_counter();//除了0进程外所有进程counter均为0, 重置counter重新调度
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



