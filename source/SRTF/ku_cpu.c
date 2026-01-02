  GNU nano 2.9.3                                                                                                     ku_cpu.c                                                                                                                

#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/linkage.h>
#include <linux/uaccess.h>
#include <linux/list.h>
#include <linux/module.h>
//  SRTF Scheduling

// structure
typedef struct {
    pid_t pid;
    int jobTime;
} job_t;

#define TIME_SLICE 4 // random
#define IDLE -1
static pid_t now = IDLE;
#define QUEUE_MAX 100
static job_t queue[QUEUE_MAX];
static int count = 0;

static bool ku_is_empty(void) { return !(count > 0); }

// push job to queue
static void ku_push(job_t newJob) {
        int w;
        w = count-1;
        if (count >= QUEUE_MAX) {
                printk(KERN_WARNING "Queue is full\n");
                return;
        }
        while (w >= 0 && queue[w].jobTime > newJob.jobTime) {
                queue[w + 1] = queue[w];
                w--;
        }
        queue[w + 1] = newJob;
        count++;
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

static bool ku_is_new_id(pid_t pid) {
    int k = 0;
    while (k < count) {
        if (queue[k].pid == pid) return false;
        k++;
    }
    return true;
}

// reorganizes queue for shortest remaining time first
static void organize_queue(void) {
        int i, j; // indices for searching 
        job_t temp; // to store when swithing
        // organize the queue based on remaining job time 
        for (i = 0; i < count - 1; i++) { // check times
                for (j = 0; j < count - i - 1; j++) {
                        if (queue[j].jobTime > queue[j + 1].jobTime) {
                                temp = queue[j];
                                queue[j] = queue[j + 1];
                                queue[j + 1] = temp;
                        }
                }
        }
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
                else {
                        printk(KERN_INFO "Working: %s\n",name);
                        return 0; // request accepted
                }
        }
        else {
                if (ku_is_new_id(newJob.pid)) ku_push(newJob);
                organize_queue(); // Call to reorganize the queue

                // Check if 'now' is still the front of the queue
                if (now == queue[0].pid) {
                        printk(KERN_INFO "Working Denied: %s\n", name); // Print if now is still the front
                } else {
                        printk(KERN_INFO "Turnover --> %s\n", name); // Print if now is not the front
                        now = queue[0].pid; // update 'now' to be the front of line
                }
        }
        return 1; // request rejected
} // os2024_ku_cpu








