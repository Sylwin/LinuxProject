#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#include <errno.h>
#include <string.h>

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
    float timeInterval = avgInterval+(1.0*rand()/RAND_MAX)*deviation*2-deviation;
    interval.it_value.tv_sec = (int)timeInterval;
    interval.it_value.tv_nsec = (timeInterval - (int)timeInterval)*1000000000;

    if( timer_settime(intervalTimerId, 0, &interval, NULL) == -1 )
        perror("timer_settime3");

    clock_gettime(CLOCK_REALTIME, &realTime);
    long sec = realTime.tv_sec;
    long nsec = realTime.tv_nsec;

    FILE *f = fopen("stempleDatownika", "a");
    if( f == NULL )
    {
        printf("error opening file");
        exit(1);
    }
    fprintf(f,"%ld %ld\n",sec, nsec);
    fclose(f);

    if( (numOfFifos > 0) || (numOfFiles > 0) )
    {
        for(int i = 0 ; i < numOfFifos; i++)
        {
            fd = open(&fifos[i], O_RDWR | O_NONBLOCK);
            write(fd, &realTime, sizeof(realTime));
        }
        for(int i = 0 ; i < numOfFiles; i++)
        {
            write(files[i], &realTime, sizeof(realTime));
        }
    }
}

int main(int argc, char* argv[])
{
    srand(time(NULL));
    clockid_t clockType = -1;
    struct itimerspec workTime;
    float time = 0;
    int check = 0;
    int opt;

    while( (opt = getopt(argc, argv, ":m:d:w:c:p:f:s:")) != -1 )
    {
        switch(opt)
        {
        case 'm':
            avgInterval = strtof(optarg,NULL);
            break;
        case 'd':
            deviation = strtof(optarg,NULL);
            break;
        case 'w':
            clockType = CLOCK_REALTIME;
            check += 1;
            time = strtof(optarg,NULL);
            memset(&workTime, 0, sizeof(workTime));
            workTime.it_value.tv_sec = (int)time;
            workTime.it_value.tv_nsec = (time - (int)time)*1000000000;
            break;
        case 'c':
            clockType = CLOCK_MONOTONIC;
            check += 1;
            time = strtof(optarg,NULL);
            memset(&workTime, 0, sizeof(workTime));
            workTime.it_value.tv_sec = (int)time;
            workTime.it_value.tv_nsec = (time - (int)time)*1000000000;
            break;
        case 'p':
            clockType = CLOCK_PROCESS_CPUTIME_ID;
            check += 1;
            time = strtof(optarg,NULL);
            memset(&workTime, 0, sizeof(workTime));
            workTime.it_value.tv_sec = (int)time;
            workTime.it_value.tv_nsec = (time - (int)time)*1000000000;
            break;
        case 'f':
            strcpy(&fifos[numOfFifos++], optarg);
            break;
        case 's':
            files[numOfFiles++] = strtol(optarg, NULL, 10);
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

    if( avgInterval <= 0 || deviation < 0 || time < 0 )
    {
        printf("Usage : %s -mFLOAT [-dFLOAT] [-w/c/pFLOAT] -fSTRING -sINT\n",argv[0]);
        if(avgInterval < 0) printf("-m value must be positive\n");
        if(deviation < 0 )  printf("-d value must be positive\n");
        if(time < 0)        printf("-w/c/p value must be positive\n");
        return 0;
    }
    if( deviation >= avgInterval )
    {
        printf("Deviation must be smaller than average value!\n");
        return 0;
    }
    if( (numOfFifos+numOfFiles) == 0 )
    {
        printf("You must specify at least one option -f or -s\n");
        return 0;
    }
    if( check > 1 )
    {
        printf("You can specify only one -w, -c or -p option\n");
        return 0;
    }

    if( (workTime.it_value.tv_sec == 0) && (workTime.it_value.tv_nsec == 0) )
        //return 0;
        raise(SIGKILL);

    FILE *f = fopen("stempleDatownika", "w");

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &sigHandler;
    sigemptyset(&sa.sa_mask);
    if( sigaction(SIGALRM, &sa, NULL) == -1 )
        perror("sigaction");

    struct sigevent se;
    memset(&se, 0, sizeof(se));
    se.sigev_notify = SIGEV_SIGNAL;
    se.sigev_signo = SIGALRM;
    se.sigev_value.sival_ptr = &intervalTimerId;
    if( timer_create(CLOCK_REALTIME, &se, &intervalTimerId) == -1 )
        perror("timer_create1");

    float timeInterval = avgInterval+(1.0*rand()/RAND_MAX)*deviation*2-deviation;
    interval.it_value.tv_sec = (int)timeInterval;
    interval.it_value.tv_nsec = (timeInterval - (int)timeInterval)*1000000000;

    if( timer_settime(intervalTimerId, 0, &interval, NULL) == -1 )
        perror("timer_settime1");

    if( clockType != -1 )
    {
        struct sigevent endEvent;
        memset(&endEvent, 0, sizeof(endEvent));
        endEvent.sigev_notify = SIGEV_SIGNAL;
        endEvent.sigev_signo = SIGKILL;
        endEvent.sigev_value.sival_ptr = &workTimerId;
        if( timer_create(clockType, &endEvent, &workTimerId) == -1 )
            perror("timer_create2");
        if( timer_settime(workTimerId, 0, &workTime, NULL) == -1 )
            perror("timer_settime2");
    }

    while(1)
    {
        //waiting for signal
        pause();
    }

    return 0;
}
