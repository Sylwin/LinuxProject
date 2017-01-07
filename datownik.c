#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

float avgInterval = 0;
float deviation = 0;
timer_t intervalTimerId;
timer_t workTimerId;
struct itimerspec interval;
struct timespec realTime;
char* fifos[20];
int numOfFifos = 0;
int files[10];
int numOfFiles = 0;
int fd;

void sigHandler(int sig)
{
    //random value between: avgInterval-deviation and avgInterval+deviation
    float timeInterval = avgInterval-deviation+(rand()/(RAND_MAX+1.0))*(deviation+avgInterval);

    interval.it_value.tv_sec = floor(timeInterval);
    interval.it_value.tv_nsec = (timeInterval - floor(timeInterval))*1000000000;

    if (timer_settime(intervalTimerId, 0, &interval, NULL) == -1)
        perror("timer_settime3");

    clock_gettime(CLOCK_REALTIME, &realTime);
    if((numOfFifos > 0) || (numOfFiles > 0))
    {
        for(int i = 0 ; i < numOfFifos; i++)
        {
            //mkfifo(&fifos[i], 0666);
            fd = open(&fifos[i], O_WRONLY | O_NONBLOCK);

            clock_gettime(CLOCK_REALTIME,&realTime);
            write(fd, &realTime, sizeof(realTime));// == -1 )
             //   perror("writing to fifo");

            close(fd);
        }
        for(int i = 0 ; i < numOfFiles; i++)
        {
            write(files[i], &realTime, sizeof(realTime));// == -1 )
             //   perror("writing to files");
        }
    }
}

int main(int argc, char* argv[])
{
    srand(time(NULL));
    clockid_t clockType = -1;
    struct itimerspec workTime;
    float time = 0;
    int opt;

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
            workTime.it_value.tv_sec = floor(time);
            workTime.it_value.tv_nsec = (time - floor(time))*1000000000;
            break;
        case 'c':
            clockType = CLOCK_MONOTONIC;
            time = strtof(optarg,NULL);
            workTime.it_value.tv_sec = floor(time);
            workTime.it_value.tv_nsec = (time - floor(time))*1000000000;
            break;
        case 'p':
            clockType = CLOCK_PROCESS_CPUTIME_ID;
            time = strtof(optarg,NULL);
            workTime.it_value.tv_sec = floor(time);
            workTime.it_value.tv_nsec = (time - floor(time))*1000000000;
            break;
        case 'f':
            strcpy(&fifos[numOfFifos++], optarg);
            break;
        case 's':
            files[numOfFiles++] = atoi(optarg);
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

   // for(int i = 0; i < numOfFifos; i++)
   //     printf("fifo: %s\n", &fifos[i]);

   // for(int i = 0; i < numOfFiles; i++)
   //     printf("files: %d\n", files[i]);

   // if(argc < 3)
   // {
   //     printf("Usage : %s -mFLOAT [-dFLOAT] [-w/c/pFLOAT] -fSTRING -sINT\n",argv[0]);
   //     return 0;
   // }

    if(avgInterval <= 0 || deviation < 0 || time < 0)
    {
        printf("Usage : %s -mFLOAT [-dFLOAT] [-w/c/pFLOAT] -fSTRING -sINT\n",argv[0]);
        if(avgInterval < 0) printf("-m value must be positive\n");
        if(deviation < 0 )  printf("-d value must be positive\n");
        if(time < 0)        printf("-w/c/p value must be positive\n");
        return 0;
    }
    if(deviation >= avgInterval)
    {
        printf("Deviation must be bigger than average value!\n");
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
    interval.it_value.tv_sec = floor(timeInterval);
    interval.it_value.tv_nsec = (timeInterval - floor(timeInterval))*1000000000;

    if (timer_settime(intervalTimerId, 0, &interval, NULL) == -1)
        perror("timer_settime1");

    if(clockType != -1)
    {
        struct sigevent endEvent;
        memset(&endEvent, 0, sizeof(endEvent));
        endEvent.sigev_notify = SIGEV_SIGNAL;
        endEvent.sigev_signo = SIGKILL;
        endEvent.sigev_value.sival_ptr = &workTimerId;
        if (timer_create(clockType, &endEvent, &workTimerId) == -1)
            perror("timer_create2");
        if (timer_settime(workTimerId, 0, &workTime, NULL) == -1)
            perror("timer_settime2");
    }

    while(1)
    {
        //waiting for signal
        pause();
    }

    return 0;
}
