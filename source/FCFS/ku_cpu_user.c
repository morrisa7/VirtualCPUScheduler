
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

// FCFS Scheduling:
#define PID_PRINT 338
#define KU_CPU 339

int main(int argc, char ** argv){
        int jobTime, delayTime;
        char name[4]; // or char name[4] = "";
        int wait = 0;

        if (argc < 4 ) {
                printf("\nInsufficient Arguments..\n");
                return 1;
        }

        /* first argument: job time in seconds (required)
           second argument: delay time before execution in seconds (required)
           third argument: process name (required)
        */

        jobTime = atoi(argv[1]);
        delayTime = atoi(argv[2]);
        strcpy(name,argv[3]);
        //syscall(PID_PRINT, name);

        sleep(delayTime);

        struct timeval start, first_cpu_time, end;
        gettimeofday(&start, NULL);
        int first_time = 1;

        printf("\nProcess %s : I will use CPU by %ds. \n", name, jobTime);
        jobTime *= 10; // execute system call in every 0.1 second

        while (jobTime) {
                // if request is rejected, increase wait time
                if (!syscall(KU_CPU, name, jobTime)) {
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

        syscall(KU_CPU, name, 0);

        gettimeofday(&end, NULL);
        double responseTime = (first_cpu_time.tv_sec - start.tv_sec) + (first_cpu_time.tv_usec - start.tv_usec) / 1000000.0;

        printf("\nProcess %s : Finish! My response time is %ds and my total wait time is %ds. ", name, (int)(responseTime+0.5), (wait+5)/10);
        return 0;
}




