#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

char fifo[20];
int fd;
struct timespec buf;

int main(int argc, char* argv[])
{
    float time;
    struct timespec t;
    int opt;

    while ((opt = getopt(argc, argv, ":d:")) != -1)
    {
        switch (opt)
        {
        case 'd':
            time = strtof(optarg, NULL);
            t.tv_sec = floor(time);
            t.tv_nsec = (time - floor(time))*1000000000;
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

    while(1)
    {
//        nanosleep(&t, NULL);
//        printf("I'm still working\n");
        if( (fd = open(fifo, O_RDONLY)) == -1 )
            return 0;
        read(fd, &buf, sizeof(buf));
        printf("Received: %ld.%.9ld\n", buf.tv_sec, buf.tv_nsec);
    }

    close(fd);

    return 0;
}
