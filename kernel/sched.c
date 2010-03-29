/* 
 * Contains scheduling primitives such as the scheduler itself,
 * wait queues and eventually semaphores...
*/

#define DEBUG_MODULE 0
#include <kernel.h>
#include <task.h>
#include <arch/processor.h>

static LIST_HEAD(runq);

/* Sleep the current task on a wait queue */
void sleep_on(struct waitq *q)
{
	struct task *t = __this_task;
	long flags;

	lock_irq(flags);

	/* Remove from run queue */
	t->state = TASK_SLEEPING;
	list_del(&t->list);

	/* Add to wait queue */
	list_add_tail(&t->list, &q->list);

	dprintk("sched: Task %s going to sleep\n", t->name);
	sched();
	unlock_irq(flags);
}

/* Wake up all sleepers on a wait queue */
void wake_up(struct waitq *q)
{
	struct task *t, *tmp;
	long flags;

	lock_irq(flags);

	/* Add all waiting tasks to runq */
	list_for_each_entry_safe(t, tmp, &q->list, list) {
		list_del(&t->list);
		task_to_runq(t);
	}

	unlock_irq(flags);

	sched();
}

/* The task that becomes the idle task needs to call this function - interrupts
 * should be disabled by the caller */
static struct task *idle_task;
void sched_init(void)
{
	idle_task =  __this_task;
	idle_task->state = TASK_RUNNING;
	idle_task->name = "[idle]";
	idle_task->pid = 0;
}

/* Put a task on the runq */
void task_to_runq(struct task *t)
{
	long flags;
	lock_irq(flags);

	t->state = TASK_READY;
	list_add_tail(&t->list, &runq);
	dprintk("sched: sleeper %s back to runq\n", t->name);

	unlock_irq(flags);
}

static void task_push_word(struct task *tsk, uint32_t word)
{
	tsk->t.esp -= sizeof(word);
	*(uint32_t *)tsk->t.esp = word;
}

_noreturn _asmlinkage
static void kthread_init(void (*thread_func)(void *), void *priv)
{
	sti();
	printk("kthread_init: thread_func=%p priv=%p\n", thread_func, priv);
	(*thread_func)(priv);
	printk("FIXME: call __exit() when implemented\n");
	idle_task_func();
}

int kernel_thread(const char *proc_name,
			void (*thread_func)(void *),
			void *priv)
{
	struct task *tsk;
	static pid_t pid = 1;
	static pid_t ret;
	long eflags;

	tsk = alloc_page();
	if ( NULL == tsk )
		return -1;

	ret = pid++;
	tsk->pid = ret;
	tsk->name = proc_name;
	tsk->t.eip = (uint32_t)kthread_init;
	tsk->t.esp = (uint32_t)tsk + PAGE_SIZE;

	/* push arguments to kthread init */
	task_push_word(tsk, (uint32_t)priv);
	task_push_word(tsk, (uint32_t)thread_func);

	/* then why is this needed? */
	get_eflags(eflags);
	task_push_word(tsk, eflags);

	task_to_runq(tsk);
	return tsk->pid;
}

/* Crappy round-robin type scheduler, just picks the
 * next task on the run queue
 */
static struct task *get_next_task(struct task *current)
{
	struct task *ret;

	if ( list_empty(&runq) ) {
		ret = idle_task;
		printk("Idling...\n");
	}else{
		ret = list_entry(runq.next, struct task, list);
		if ( current == ret ) {
			list_move_tail(&current->list, &runq);
			ret = list_entry(runq.next, struct task, list);
		}
	}

	BUG_ON(ret->state == TASK_SLEEPING);
	return ret;
}

void sched(void)
{
	struct task *current;
	struct task *head;
	long flags;

	lock_irq(flags);

	current = __this_task;
	head = get_next_task(current);
	if ( current == head )
		goto out;

	dprintk("sched: switch from %s to %s\n", current->name, head->name);
	current->state = TASK_READY;
	head->state = TASK_RUNNING;
	switch_task(current, head);

out:
	unlock_irq(flags);
}
