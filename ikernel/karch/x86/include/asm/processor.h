#ifndef PROCESSOR_H_
#define PROCESSOR_H_

#define task_top_of_stack(task)		((unsigned long)((char *)task->stack + KSTACK_SIZE))

#endif // PROCESSOR_H_
