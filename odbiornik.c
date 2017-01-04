#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
//#include <time.h>
//#include <math.h>
#include <errno.h>
#include <string.h>
#include <poll.h>
#include "common.h"

char fifo[20];
int fd;
timer_t timerId;
float controlTime = 0;
struct itimerspec t;
struct timespec currentLocalTime;

void handler(int sig)
{
    writingTimeToTimespec(controlTime, &t.it_value);
    if (timer_settime(timerId, 0, &t, NULL) == -1)
        perror("timer_settime1");

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &currentLocalTime);
    long curLocalTimeSec = currentLocalTime.tv_sec;
    long curLocalTimeNSec = currentLocalTime.tv_nsec;
    printf("I'm still working\nCurrent local time %ld.%.9ld\n\n", curLocalTimeSec, curLocalTimeNSec);
}

int main(int argc, char* argv[])
{
    int opt;

    while ((opt = getopt(argc, argv, ":d:")) != -1)
    {
        switch (opt)
        {
        case 'd':
            controlTime = strtof(optarg, NULL);
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

    if(argc<2)
    {
        printf("Usage: %s [-d float] path\n", argv[0]);
        return 0;
    }

    strcpy(fifo, argv[optind]);
    mkfifo(fifo, 0666);

    if(controlTime)
    {
        //register handler
        struct sigaction sa;
        memset(&sa, 0, sizeof(sa));//struct sigaction));
        sa.sa_handler = &handler;
        sigemptyset(&sa.sa_mask);   //no signal would be blocked
        if (sigaction(SIGALRM, &sa, NULL) == -1)
            perror("sigaction");

        //create timer
        struct sigevent se;
        memset(&se, 0, sizeof(se));//struct sigevent));
        se.sigev_notify = SIGEV_SIGNAL;
        se.sigev_signo = SIGALRM;
        se.sigev_value.sival_ptr = &timerId;
        if (timer_create(CLOCK_REALTIME, &se, &timerId) == -1)
            perror("timer_create");

        writingTimeToTimespec(controlTime, &t.it_value);
        if (timer_settime(timerId, 0, &t, NULL) == -1)
            perror("timer_settime1");
    }

    int fd = open(fifo, O_RDWR);

    struct pollfd fdst;
    fdst.fd = fd;
    fdst.events = POLLIN;
    fdst.revents = 0;

    int res;

    while(1)
    {
        //pause();
        res = poll(&fdst,1,0);

        if( res == 1)
        {
            if(fdst.revents & POLLIN)
            {
//              if((fdst.revents & POLLERR) || (fdst.revents & POLLNVAL))
//                      break;
                struct timespec buf;
                struct timespec currentTime;
                read(fdst.fd, &buf, sizeof(buf));
                long recSec = buf.tv_sec;
                long recNSec = buf.tv_nsec;

                clock_gettime(CLOCK_REALTIME,&currentTime);
                long curSec = currentTime.tv_sec;
                long curNSec = currentTime.tv_nsec;

                printf("Received:   %ld.%.9ld\n", recSec, recNSec);
                printf("Current:    %ld.%.9ld\n", curSec, curNSec);
                printf("Difference:          %ld.%.9ld\n\n", curSec-recSec, curNSec-recNSec);
            }
            else
            {
                if((fdst.revents & POLLERR) || (fdst.revents & POLLNVAL))
                    break;
            }
        }
    }
    close(fd);

    return 0;
}
