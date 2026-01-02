#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/linkage.h>
#include <linux/uaccess.h>
#include <linux/list.h>
#include <linux/module.h>

// priority scheduling

typedef struct {
    pid_t pid;
    int jobTime;
    int priority;
} job_t;

#define QUEUE_MAX 100
#define IDLE -1

static pid_t now = IDLE;
static job_t queue[QUEUE_MAX];
static int count = 0;

static bool ku_is_empty(void) { return (count == 0); }
static bool ku_is_full(void) { return (count == QUEUE_MAX); }

// add job to queue
static void ku_push(job_t newJob) {
    int j = 0;
    if (ku_is_full()) {
        printk(KERN_WARNING "The queue is full.\n");
        return;
    }
    // insert based on priority as well to save time searching in other functions
    for (j = count - 1; j >= 0 && newJob.priority < queue[j].priority; j--) {
        queue[j + 1] = queue[j];
    }
    queue[j + 1] = newJob;
    count++;
}

// remove job from queue
static job_t ku_pop(void) {
    job_t to_pop = {IDLE, 0, 0};
    if (ku_is_empty()) return to_pop;
    to_pop = queue[0]; // store first item
    memmove(queue, queue + 1, (count - 1) * sizeof(job_t)); // shift items leftwards
    count--; 
    return to_pop; // return the first item of the list we removed
}

static bool ku_is_new_id(pid_t pid) { // check job isnt already in queue
    int k = 0;
    while (k < count) {
        if (queue[k].pid == pid) return false;
        k++;
    }
    return true;
}

static bool is_highest_priority(job_t aJob) { // iterate to check priorities
    int h; 
    for (h = 0; h < count; h++) { // search thru jobs check not itself, check priority 
        if (queue[h].pid != aJob.pid && aJob.priority >= queue[h].priority) {
            return false; // false if not highest priority 
        }
    }
    return true; // true if highest priority 
}

SYSCALL_DEFINE3(os2024_ku_cpu, char*, name, int, jobTime, int, priority) {
    job_t newJob = {current->pid, jobTime, priority};
    job_t tempJob; // for storing is necessary

    if (now == IDLE) now = newJob.pid; 

    if (now == newJob.pid) {
        if (jobTime == 0) {
            printk(KERN_INFO "Process Finished: %s\n", name);
            if (ku_is_empty()) now = IDLE;
            else now = ku_pop().pid;
        } else { // jobTime > 0
            if (is_highest_priority(newJob)) { // check if highest priority
                printk(KERN_INFO "Working: %s\n", name); // if yes, work it
                return 0; // request accepted
            } else { // if not highest priority then reschedule 
                printk(KERN_INFO "Reschedule ----> %s\n", name);
                tempJob = newJob; // store the job
                ku_pop(); // remove it from the front
                now = queue[0].pid; // change now to next in queue
                ku_push(tempJob); // push lower priority job to back of the line
                return 1; // rejected
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
    printk(KERN_INFO "Process name: %s pid: %d\n", name, pid);
    return 0;
} // os2024_pid_print