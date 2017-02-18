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
#include <poll.h>

char fifo[20];
int fd;
timer_t controlTimerId;
float controlTime = 0;
struct itimerspec control;
struct timespec currentLocalTime;
int res;

void handler(int sig)
{
    control.it_value.tv_sec = (int)controlTime;
    control.it_value.tv_nsec = (controlTime - (int)controlTime)*1000000000;
    if( timer_settime(controlTimerId, 0, &control, NULL) == -1 )
        perror("timer_settime1");

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &currentLocalTime);
    printf("I'm still working\nCurrent local time: %ld.%ld\n\n", currentLocalTime.tv_sec, currentLocalTime.tv_nsec);
}

int main(int argc, char* argv[])
{
    int opt;

    while( (opt = getopt(argc, argv, ":d:")) != -1 )
    {
        switch(opt)
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

    if( argc < 2 )
    {
        printf("Usage: %s [-dFLOAT] path\n", argv[0]);
        return 0;
    }
    if( controlTime < 0 )
    {
        printf("-d value must be positive\n");
        return 0;
    }

    strcpy(fifo, argv[optind]);

    printf("%s\n", fifo);

    if( controlTime > 0 )
    {
        struct sigaction sa;
        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = &handler;
        sigemptyset(&sa.sa_mask);
        if( sigaction(SIGALRM, &sa, NULL) == -1 )
            perror("sigaction");

        struct sigevent se;
        memset(&se, 0, sizeof(se));
        se.sigev_notify = SIGEV_SIGNAL;
        se.sigev_signo = SIGALRM;
        se.sigev_value.sival_ptr = &controlTimerId;
        if( timer_create(CLOCK_REALTIME, &se, &controlTimerId) == -1 )
            perror("timer_create");

        control.it_value.tv_sec = (int)controlTime;
        control.it_value.tv_nsec = (controlTime - (int)controlTime)*1000000000;
        if( timer_settime(controlTimerId, 0, &control, NULL) == -1 )
            perror("timer_settime1");
    }


    fd = open(fifo, O_RDWR);

    struct pollfd fs;
    fs.fd = fd;
    fs.events = POLLIN;
    fs.revents = 0;

    while(1)
    {
        res = poll(&fs,1,-1);
        if(fs.revents & POLLIN)
        {
            struct timespec buf;
            struct timespec currentTime;
            read(fs.fd, &buf, sizeof(buf));
            long recSec = buf.tv_sec;
            long recNSec = buf.tv_nsec;

            clock_gettime(CLOCK_REALTIME,&currentTime);
            long curSec = currentTime.tv_sec;
            long curNSec = currentTime.tv_nsec;

            printf("Reading from: %s\nDifference:         %ld.%.9ld\n\n", fifo, curSec-recSec, curNSec-recNSec);
        }
        else if( (fs.revents & POLLNVAL) || (fs.revents & POLLERR))
            break;
    }

    close(fd);
    return 0;
}
