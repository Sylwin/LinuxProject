#include "common.h"

void writingTimeToTimespec(float time, struct timespec *t)
{
    t->tv_sec = floor(time);
    t->tv_nsec = (time - floor(time))*1000000000;
}
