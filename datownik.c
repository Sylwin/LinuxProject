#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <signal.h>
//#include <time.h>
//#include <math.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "common.h"

float avgInterval = 0;
float deviation = 0;
timer_t intervalTimerId;
timer_t workTimerId;
struct itimerspec t2;
struct timespec t3;
char* fifos[20];
int numOfFifos = 0;
int files;
int numOfFiles = 0;
int fd;

void sigHandler(int signum)
{
    //random value between: avarage-deviation and avgInterval+deviation
    float timeInterval = avgInterval-deviation+(rand()/(RAND_MAX+1.0))*(deviation+avgInterval);
    writingTimeToTimespec(timeInterval, &t2.it_value);

    if (timer_settime(intervalTimerId, 0, &t2, NULL) == -1)
        perror("timer_settime3");

    if(numOfFifos)
    {
        for(int i = 0 ; i < numOfFifos; i++)
        {
            mkfifo(&fifos[i], 0666);
            fd = open(&fifos[i], O_WRONLY | O_NONBLOCK);

            clock_gettime(CLOCK_REALTIME,&t3);
            write(fd, &t3, sizeof(t3));// == -1 )
             //   perror("writing to fifo");

            close(fd);
        }
    }
   // if(numOfFiles)
   // {
   //     for(int i = 0 ; i < numOfFiles; i++)
   //     {
   //         mkfifo(files[i], 0666);
   //         fd = open(files[i], O_WRONLY);// | O_NONBLOCK);

   //         clock_gettime(CLOCK_REALTIME,&t3);
   //         if( write(fd, &t3, sizeof(t3)) == -1 )
   //             perror("writing to fifo");

   //         close(fd);
   //     }

   // }
}

int main(int argc, char* argv[])
{
    srand(time(NULL));
    clockid_t clockType = -1;
    struct itimerspec t1;
    float time;

    int opt;
    int index;

    while ((opt = getopt(argc, argv, ":m:d:w:c:p:f:s:")) != -1)
    {
        switch (opt)
        {
        case 'm':
            avgInterval = strtof(optarg,NULL);
            break;
        case 'd':
            deviation = strtof(optarg,NULL);
            break;
        case 'w':
            clockType = CLOCK_REALTIME;
            time = strtof(optarg,NULL);
            writingTimeToTimespec(time, &t1.it_value);
            break;
        case 'c':
            clockType = CLOCK_MONOTONIC;
            time = strtof(optarg,NULL);
            writingTimeToTimespec(time, &t1.it_value);
            break;
        case 'p':
            clockType = CLOCK_PROCESS_CPUTIME_ID;
            time = strtof(optarg,NULL);
            writingTimeToTimespec(time, &t1.it_value);
            break;
        case 'f':
            //optind--;
            //for( ;optind < argc && *argv[optind] != '-'; optind++){
            //    strcpy(&fifos[numOfFifos++],optarg);
            //}
            strcpy(&fifos[numOfFifos++], optarg);
            break;
        case 's':
            //optind--;
            //for( ;optind < argc && *argv[optind] != '-'; optind++){
            //    files[numOfFiles++] = atoi(optarg);
            //}
            files = atoi(optarg);
            break;
        case ':':
            fprintf(stderr, "%s: option '-%c' requires an argument\n", argv[0], optopt);
            break;
        case '?':
        default:
            fprintf(stderr, "%s: option '-%c' is invalid: ignored\n", argv[0], optopt);
            break;
        }
    }

    for(int i = 0; i < numOfFifos; i++)
        printf("fifo: %s\n", &fifos[i]);

    //for(int i = 0; i < numOfFiles; i++)
      //  printf("fifo: %s\n", &files[i]);

    if(argc < 3)
    {
        printf("Usage : %s -m float [-d float] -w/c/p float -f string -s int\n",argv[0]);
        return 0;
    }
    if(avgInterval < 0)
    {
        printf("Usage : %s -m float [-d float] -w/c/p float -f string -s int\n-m option's value must be positive\n",argv[0]);
        return 0;
    }

    //register handler
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &sigHandler;
    sigemptyset(&sa.sa_mask);   //no signal would be blocked
    if (sigaction(SIGALRM, &sa, NULL) == -1)
        perror("sigaction");

    //create timer
    struct sigevent se;
    memset(&se, 0, sizeof(se));
    se.sigev_notify = SIGEV_SIGNAL;
    se.sigev_signo = SIGALRM;
    se.sigev_value.sival_ptr = &intervalTimerId;
    if (timer_create(CLOCK_REALTIME, &se, &intervalTimerId) == -1)
        perror("timer_create1");

    float timeInterval = avgInterval-deviation+(rand()/(RAND_MAX+1.0))*(deviation+avgInterval);
    writingTimeToTimespec(timeInterval, &t2.it_value);

    if (timer_settime(intervalTimerId, 0, &t2, NULL) == -1)
        perror("timer_settime1");

    if(clockType != -1)
    {
        struct sigevent sevProgramEnd;
        sevProgramEnd.sigev_notify = SIGEV_SIGNAL;
        sevProgramEnd.sigev_signo = SIGKILL;
        sevProgramEnd.sigev_value.sival_ptr = &workTimerId;
        if (timer_create(clockType, &sevProgramEnd, &workTimerId) == -1)
            perror("timer_create2");
        if (timer_settime(workTimerId, 0, &t1, NULL) == -1)
            perror("timer_settime2");
    }

    while(1)
    {
        //waiting for signal
        pause();
    }


    close(fd);
    return 0;
}
