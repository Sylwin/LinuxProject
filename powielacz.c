#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdbool.h>
#include <poll.h>

struct Fifo
{
    char path[50];
    int fileDescriptor;
    bool isOpened;
};

char filesNamePattern[20];
char diagnosticFile[20];
int numOfFifos = 0;
struct Fifo* fifos;
int res;
int closed = 0;

int main(int argc, char* argv[])
{
    int opt;

    while ((opt = getopt(argc, argv, ":p:c:L:")) != -1)
    {
        switch (opt)
        {
        case 'p':
            strcpy(filesNamePattern, optarg);
            break;
        case 'c':
            numOfFifos = atoi(optarg);
            break;
        case 'L':
            strcpy(diagnosticFile, optarg);
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

    if(argc < 2 || numOfFifos < 1)
    {
        printf("Usage : %s -p string -c int [-L string]\n", argv[0]);
        return 0;
    }

    if( strlen(diagnosticFile) != 0 )
    {
        printf("Diagnostic information in '%s' file\n", diagnosticFile);
        int fw = open(diagnosticFile, O_WRONLY | O_CREAT, 0666);
        // replace standard output with output file
        if( dup2(fw, 1) == -1 )
            perror("dup");
    }

    fifos = (struct Fifo*)malloc(numOfFifos * sizeof(struct Fifo));

    for(int i = 0; i < numOfFifos; i++)
    {
        sprintf(fifos[i].path,"%s%d",filesNamePattern,i + 1);
        fifos[i].isOpened = false;

        struct stat sb;
        if (stat(fifos[i].path, &sb) == -1)
        {
            printf("%s does not exist.\n", fifos[i].path);
            closed++;
            if( closed == numOfFifos )
                return 0;
            continue;
        }
        if(!S_ISFIFO(sb.st_mode))
        {
            printf("%s is not fifo.\n", fifos[i].path);
            closed++;
            if( closed == numOfFifos )
                return 0;
            continue;
        }
        else if(!(sb.st_mode & S_IROTH))
        {
            printf("%s is not open for reading\n", fifos[i].path);
            closed++;
            if( closed == numOfFifos )
                return 0;
            continue;
        }

        if( !fifos[i].isOpened )
        {
            printf("%s ", fifos[i].path);
            int fd = open(fifos[i].path, O_RDWR | O_NONBLOCK);
            if(fd != -1)
            {
                printf("opened\n");
                fifos[i].isOpened = true;
                fifos[i].fileDescriptor = fd;
            }
            else
            {
                printf("failed\n");
                closed++;
                if( closed == numOfFifos )
                    return 0;
            }
            continue;
        }
    }

    struct pollfd fds;
    fds.fd = 0;
    fds.events = POLLIN;
    fds.revents = 0;

    while(1)
    {
        res = poll(&fds,1,-1);          // -1 -> infinite timeout
        if(fds.revents & POLLIN)
        {
            struct timespec buf;
            read(fds.fd, &buf, sizeof(buf));
            //printf("Received clock value: %ld.%ld\n", buf.tv_sec, buf.tv_nsec);
            for(int i = 0; i < numOfFifos; i++)
            {
                if(fifos[i].isOpened)
                {
                    int result = write(fifos[i].fileDescriptor, &buf, sizeof(buf));
                    if(result == -1 && errno == EAGAIN) //overflow
                    {
                        close(fifos[i].fileDescriptor);
                        printf("%s closed because of overflow\n", fifos[i].path);
                        closed++;
                    }
                }
                if( numOfFifos == closed )
                    return 0;
            }
        }
        else
        {
            if( (fds.revents & POLLNVAL) || (fds.revents & POLLERR ))
                //break;
                return 0;
        }
    }

    free(fifos);

    return 0;
}

