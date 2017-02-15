#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <poll.h>

char filesNamePattern[20];
char diagnosticFile[20];
int numOfFifos = 0;
int closed = 0;
int res;

struct Fifo
{
    char path[20];
    int fileDescriptor;
    bool isOpened;
};

struct Fifo* fifos;

int main(int argc, char* argv[])
{
    int opt;

    while( (opt = getopt(argc, argv, ":p:c:L:")) != -1 )
    {
        switch(opt)
        {
        case 'p':
            strcpy(filesNamePattern, optarg);
            break;
        case 'c':
            numOfFifos = strtol(optarg, NULL, 10);
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

    if(argc < 2)
    {
        printf("Usage : %s -pSTRING -cINT [-LSTRING]\n", argv[0]);
        return 0;
    }
    if( numOfFifos <= 0 )
    {
        printf("-c value must be positve\n");
        return 0;
    }
    if( strlen(filesNamePattern) == 0 )
    {
        printf("You must specjify file name pattern (-p option)\n");
        return 0;
    }

    if( strlen(diagnosticFile) != 0 )
    {
        printf("Diagnostic information in '%s' file\n\n", diagnosticFile);
        int fw = open(diagnosticFile, O_WRONLY | O_CREAT, 0666);
        if( dup2(fw, 1) == -1 )
            perror("dup");
    }

    fifos = (struct Fifo*)malloc(numOfFifos * sizeof(struct Fifo));

    for( int i = 0; i < numOfFifos; i++ )
    {
        sprintf(fifos[i].path,"%s%d",filesNamePattern,i + 1);
        fifos[i].isOpened = false;

        struct stat sb;
        if( stat(fifos[i].path, &sb) == -1 )
        {
            printf("%s does not exist\n", fifos[i].path);
            closed++;
            if( closed == numOfFifos )
                return 0;
            continue;
        }
        if( !S_ISFIFO(sb.st_mode) )
        {
            printf("%s is not fifo\n", fifos[i].path);
            closed++;
            if( closed == numOfFifos )
                return 0;
            continue;
        }
        if( !(sb.st_mode & S_IROTH) )
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

    struct pollfd fs;
    fs.fd = 0;
    fs.events = POLLIN;
    fs.revents = 0;

    while(1)
    {
        res = poll(&fs,1,-1);
        if(fs.revents & POLLIN)
        {
            struct timespec buf;
            read(fs.fd, &buf, sizeof(buf));
            for( int i = 0; i < numOfFifos; i++ )
            {
                if( fifos[i].isOpened )
                {
                    int result = write(fifos[i].fileDescriptor, &buf, sizeof(buf));
                    if( result == -1 && errno == EAGAIN ) //overflow
                    {
                        close(fifos[i].fileDescriptor);
                        printf("%s closed because of overflow\n", fifos[i].path);
                        closed++;
                        if( closed == numOfFifos )
                            return 0;
                    }
                }
            }
        }
        else if( (fs.revents & POLLNVAL) || (fs.revents & POLLERR ))
            break;
    }

    return 0;
}

