sudo#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/linkage.h>
#include <linux/uaccess.h>
#include <linux/list.h>
#include <linux/module.h>
//  RR Scheduling

// structures
typedef struct {
    pid_t pid;
    int jobTime;
} job_t;

#define TIME_SLICE 3 // randomly chosen
#define IDLE -1
#define QUEUE_MAX 100

static pid_t now = IDLE;
static job_t queue[QUEUE_MAX];
static int count = 0;
static int time_spent = 0; // to know when to let other jobs have a turn

static bool ku_is_empty(void) { return !(count > 0); }

// push job to queue
static void ku_push(job_t newJob) {
        count = (count < QUEUE_MAX) ? (queue[count++] = newJob, count) : (printk(KERN_WARNING "Queue is full\n"), count);
}

// pop job from queue
static job_t ku_pop(void) {
    job_t to_pop = {IDLE, 0};
    if (ku_is_empty()) return to_pop;
    to_pop = queue[0];
    memmove(queue, queue + 1, (count - 1) * sizeof(job_t)); // shift items
    count--;
    return to_pop;
}

static bool ku_is_new_id(pid_t pid) {
    int k = 0;
    while (k < count) {
        if (queue[k].pid == pid) return false;
        k++;
    }
    return true;
}

SYSCALL_DEFINE2(os2024_ku_cpu, char*, name, int, jobTime) {
    job_t newJob = {current->pid, jobTime};
    job_t tempJob; // for storing if needed later

    if (now == IDLE) {
        now = newJob.pid;
        time_spent = 0;
    }

    if (now == newJob.pid) {
        if (jobTime == 0) {
            printk(KERN_INFO "Process Finished: %s\n", name);
            if (ku_is_empty()) {
                now = IDLE;
            } else {
                tempJob = ku_pop();
                now = tempJob.pid;
                time_spent = 0;
            }
        } else { // jobTime > 0
            if (time_spent == TIME_SLICE) {
                printk(KERN_INFO "Turnover ----> %s\n", name);
                ku_push(newJob); // push current job back to queue if not finished
                if (ku_is_empty()) {
                    now = IDLE;
                } else {
                    tempJob = ku_pop(); // remove the next job from queue
                    now = tempJob.pid; // reset current job
                }
                time_spent = 0; // reset time_spent
                return 1; // request rejected, was moved to back of line
            } else { // time spent < TIME_SLICE
                printk(KERN_INFO "Working: %s\n", name);
                time_spent++;
                return 0; // request accepted
            }
        }
    } else { // new job, and CPU occupied
        if (ku_is_new_id(newJob.pid)) ku_push(newJob); // add to back of line
        printk(KERN_INFO "Working Denied: %s\n", name);
    }
    return 1; // request rejected
} // os2024_ku_cpu

SYSCALL_DEFINE1(os2024_pid_print, char*, name) {
        pid_t pid = current->pid;
        printk(KERN_INFO "Process name: %s pid: %d\n", name,pid);
        return 0;
} // os2024_pid_print