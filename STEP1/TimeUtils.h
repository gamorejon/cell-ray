#ifndef TIME_UTILS_H
#define TIME_UTILS_H
#include <sys/time.h>

long getMsTime()
{
	timeval time;

	gettimeofday(&time, NULL);

	return time.tv_sec * 1000 + time.tv_usec / 1000;
}

#endif

