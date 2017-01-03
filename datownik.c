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
struct itimerspec t2;
struct timespec t3;
char fifos[20];
int files;
int numOfFiles = 0;
int fd;

void writingTimeToTimespec(float time, struct timespec *t)
{
    t->tv_sec = floor(time);
    t->tv_nsec = (time - floor(time))*1000000000;
}

void sigHandler(int signum, siginfo_t *siginfo, void* context)
{
    //random value between: avarage-deviation and twice the deviation
    float timeInterval = avgInterval-deviation+(rand()/(RAND_MAX+1.0))*deviation*2;

    writingTimeToTimespec(timeInterval, &t2.it_value);

    if (timer_settime(intervalTimerId, 0, &t2, NULL) == -1)
        perror("timer_settime3");

    mkfifo(fifos, 0666);
    fd = open(fifos, O_WRONLY);

    clock_gettime(CLOCK_REALTIME,&t3);
    if( write(fd, &t3, sizeof(t3)) == -1 )
        perror("writing to stdout");

    close(fd);
}

int main(int argc, char* argv[])
{
    srand(time(NULL));
    clockid_t clockType;
    struct itimerspec t1;
    float time;

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
            strcpy(fifos, optarg);
            break;
        case 's':
            //optind--;
            //for( ;optind < argc && *argv[optind] != '-'; optind++){
            //    files[optind] = atoi(optarg);
            //    numOfFiles++;
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

//    printf("pid: %d\n", getpid());

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));//struct sigaction));
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = &sigHandler;
    sigemptyset(&sa.sa_mask);   //no signal would be blocked
    if (sigaction(SIGALRM, &sa, NULL) == -1)
        perror("sigaction");

    struct sigevent se;
    memset(&se, 0, sizeof(se));//struct sigevent));
    se.sigev_notify = SIGEV_SIGNAL;
    se.sigev_signo = SIGALRM;
    se.sigev_value.sival_ptr = &intervalTimerId;
    se.sigev_notify = SIGEV_SIGNAL;
    se.sigev_signo = SIGALRM;
    se.sigev_value.sival_ptr = &intervalTimerId;
    if (timer_create(CLOCK_REALTIME, &se, &intervalTimerId) == -1)
        perror("timer_create");


    float timeInterval = avgInterval-deviation+(rand()/(RAND_MAX+1.0))*deviation*2;
    writingTimeToTimespec(timeInterval, &t2.it_value);

    if (timer_settime(intervalTimerId, 0, &t2, NULL) == -1)
        perror("timer_settime1");


    struct sigevent sevProgramEnd;
    sevProgramEnd.sigev_notify = SIGEV_SIGNAL;
    sevProgramEnd.sigev_signo = SIGKILL;
    sevProgramEnd.sigev_value.sival_ptr = &workTimerId;
    if (timer_create(clockType, &sevProgramEnd, &workTimerId) == -1)
        perror("timer_create");
    if (timer_settime(workTimerId, 0, &t1, NULL) == -1)
        perror("timer_settime2");

    //alarm(3);
    while(1)
    {
        //waiting for signal
        pause();
    }

    return 0;
}
