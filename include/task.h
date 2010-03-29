#ifndef __TASK_HEADER_INCLUDED__
#define __TASK_HEADER_INCLUDED__

#include <arch/task.h>
#include <mm.h>
#include <list.h>

#define TASK_RUNNING   0
#define TASK_READY     1
#define TASK_SLEEPING  2 

#define INIT_WAITQ(name) { .list = LIST_HEAD_INIT(name.list) }
struct waitq {
	struct list_head	list;
};

/* Task descriptor */
struct task {
	struct list_head 	list;
	uint32_t		state;
	struct thread		t;
	const char		*name;
	pid_t 			pid;

	/* Filesystem info */
	struct inode		*root;
	struct inode		*cwd;
};

/* Wait-queue manipulation */
void sleep_on(struct waitq *);
void wake_up(struct waitq *);

/* Run-queue manupulation */
void task_to_runq(struct task *);

int kernel_thread(const char *proc_name,
			void (*thread_func)(void *),
			void *priv);

/* The scheduler */
void sched_init(void);
void sched(void);

void syscall_exit(void);

#endif /* __TASK_HEADER_INCLUDED__ */
