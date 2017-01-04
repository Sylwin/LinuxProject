#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#include <errno.h>
#include <string.h>

char filesNamePattern[20];
char diagnosticPath[20];
int numberOfFifos;

int main(int argc, char* argv[])
{
    int opt;

    while ((opt = getopt(argc, argv, "p:c:L:")) != -1)
    {
        switch (opt)
        {
        case 'p':
            strcpy(filesNamePattern, optarg);
            break;
        case 'c':
            numberOfFifos = atoi(optarg);
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

    if(argc < 2)
    {
        printf("Usage : %s -p string -c int [-L string]\n", argv[0]);
        return 0;
    }

    

    return 0;
}
