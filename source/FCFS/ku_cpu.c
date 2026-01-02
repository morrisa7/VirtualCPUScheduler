                                                                                                                         Modified  

#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/linkage.h>
#include <linux/uaccess.h>
#include <linux/list.h>
#include <linux/module.h>
//  FCFS Scheduling
// structure
typedef struct {
    pid_t pid;
    int jobTime; // later add priority when needed 
} job_t;

#define TIME_SLICE 4 // random
#define IDLE -1
static pid_t now = IDLE;
#define QUEUE_MAX 100
static job_t queue[QUEUE_MAX];
static int count = 0;

static bool ku_is_empty(void) { return !(count > 0); }

// push job to queue
static void ku_push(job_t newJob) { // ternary operator used to save space 
  count = (count < QUEUE_MAX) ? (queue[count++] = newJob, count) : (printk(KERN_WARNING "Queue is full\n"), count);
}

// pop job from queue
static pid_t ku_pop(void) {
    pid_t pid;
    if (ku_is_empty()) return IDLE;
    pid = queue[0].pid;
    memmove(queue, queue + 1, (count - 1) * sizeof(job_t)); // shift items
    count--;
    return pid;
}

static bool ku_is_new_id(pid_t pid) { // check isnt already in queue 
    int i = 0;
    while (i < count) {
        if (queue[i].pid == pid) return false;
        i++;
    }
    return true;
}

SYSCALL_DEFINE1(os2024_pid_print, char*, name) {
        pid_t pid = current->pid;
        printk(KERN_INFO "Process name: %s pid: %d\n", name,pid);
        return 0;
} // os2024_pid_print

SYSCALL_DEFINE2(os2024_ku_cpu, char*, name, int, jobTime) {
        job_t newJob = {current->pid, jobTime};

        if (now == IDLE) now = newJob.pid;

        if (now == newJob.pid) {
                if (jobTime == 0) {
                        printk(KERN_INFO "Process Finished: %s\n", name);
                        if (ku_is_empty()) now = IDLE;
                        else now = ku_pop();
                }
                else { // job time remains ... 
                        printk(KERN_INFO "Working: %s\n",name);
                        return 0; // request accepted
                }
        }
        else {
                if (ku_is_new_id(newJob.pid)) ku_push(newJob);
            printk(KERN_INFO "Working Denied: %s\n",name);
        }
        return 1; // request rejected
} // os2024_ku_cpu



