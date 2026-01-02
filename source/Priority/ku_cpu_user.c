#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

// Priority Scheduling User
#define PID_PRINT 338
#define KU_CPU 339

int main(int argc, char ** argv) {
    int jobTime, delayTime, priority;
    char name[4];
    int wait = 0;

    if (argc < 5) { // now allow priorityy argument
        printf("\nInsufficient Arguments..\n");
        return 1;
    }

    /* first argument: job time in seconds (required)
       second argument: delay time before execution in seconds (required)
       third argument: process name (required)
       fourth argument: priority (required)
    */
    jobTime = atoi(argv[1]);
    delayTime = atoi(argv[2]);
    strcpy(name, argv[3]);
    priority = atoi(argv[4]); // store priority noew given

    sleep(delayTime);

    struct timeval start, first_cpu_time, end;
    gettimeofday(&start, NULL);
    int first_time = 1;

    printf("\nProcess %s : I will use CPU for %ds.\n", name, jobTime);
    jobTime *= 10; // execute system call in every 0.1 second

    while (jobTime) {
        // if request is rejected, increase wait time
        if (!syscall(KU_CPU, name, jobTime, priority)) {
            if (first_time) {
                gettimeofday(&first_cpu_time, NULL);
                first_time = 0;
            }
            jobTime--;
        } else {
            wait++;
        }
        usleep(100000); // delay 0.1 second
    }
    syscall(KU_CPU, name, 0, priority, 3); 

    gettimeofday(&end, NULL);
    double responseTime = (first_cpu_time.tv_sec - start.tv_sec) + (first_cpu_time.tv_usec - start.tv_usec) / 1000000.0;

    printf("\nProcess %s : Finish! My response time is %ds and my total wait time is %ds.\n", name, (int)(responseTime + 0.5), (wait + 5) / 10);
    return 0;
}
