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
    bool isFull;
};

char filesNamePattern[20];
char diagnosticPath[20];
int numOfFifos = 0;
struct Fifo* fifos;
int res;

void beginHandler(int sig)
{
    for(int i = 0; i < numOfFifos; i++)
    {
        struct stat sb;
        if (stat(fifos[i].path, &sb) == -1)
        {
            printf("%s does not exist.\n", fifos[i].path);
            continue;
        }
        if(!S_ISFIFO(sb.st_mode))
        {
            printf("%s is not fifo.\n", fifos[i].path);
            continue;
        }
        else if(!(sb.st_mode & S_IROTH))
        {
            printf("%s does not open for reading\n", fifos[i].path);
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
            }
            continue;
        }

        if( fifos[i].isFull )
        {
            printf("%s buffer is full, closing.\n", fifos[i].path);
            close(fifos[i].fileDescriptor);
            break;
        }
    }
}

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
            strcpy(diagnosticPath,optarg);
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

    if( strlen(diagnosticPath) != 0 )
    {
        int desc = open(diagnosticPath, O_WRONLY | O_CREAT, 0666);
        dup2(desc, 1);
    }

    fifos = (struct Fifo*)malloc(numOfFifos * sizeof(struct Fifo));


    for(int i = 0; i < numOfFifos;i++)
    {
      //  printf("%s\n", diagnosticPath);
        sprintf(fifos[i].path,"%s%d",filesNamePattern,i);
        fifos[i].isOpened = false;
        fifos[i].isFull = false;
    }

    struct sigaction sa;
    memset( &sa, 0, sizeof(sa));
    sa.sa_handler = &beginHandler;
    sigemptyset(&sa.sa_mask);
    if( (sigaction(SIGALRM, &sa, NULL)) == -1 )
        perror("sigaction");

    struct pollfd fds;
    fds.fd = 0; //stdin
    fds.events = POLLIN;
    fds.revents = 0;

    raise(SIGALRM);

   // int res;
    while(1)
    {
       // raise(SIGALRM);
        res = poll(&fds,1,-1);          // -1 -> infinite timeout
        if(fds.revents & POLLIN)
        {
            struct timespec buffer;
            read(fds.fd,&buffer,sizeof(buffer));
            printf("%ld.%ld\n", buffer.tv_sec, buffer.tv_nsec);
            for(int i = 0; i < numOfFifos; i++)
            {
                if(fifos[i].isOpened)
                {
                    int result = write(fifos[i].fileDescriptor, &buffer,sizeof(buffer));
                    if(result == -1 && errno == EAGAIN)
                    {
                        fifos[i].isFull = true;
                        close(fds.fd);
                    }
                }
              //  if(fifos[i].isFull)
              //  {
              //      close(fds.fd);
              //  }
            }
        }
        else
        {
            if( (fds.revents & POLLNVAL) || (fds.revents & POLLERR ))
                break;
        }
    }

    free(fifos);

    return 0;
}

