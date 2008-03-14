/*
 * file: timer.c
 *
 * description: timer code for Windows and Linux
 * 
 * authors: Institut fuer Visualisierung und Interaktive Systeme, Universitaet Stuttgart
 *
 */

#include <stdio.h>

#ifdef WIN32
#	include <windows.h>
#else
#	include <sys/time.h>
#endif

#include "timer.h"

#ifdef WIN32
	double timer()
	{
		LARGE_INTEGER timerFreq, timerCount;

		if (! QueryPerformanceFrequency(&timerFreq)) {
			fprintf(stderr, "determing timer frequency failed\n");
			exit(1);
		}
		QueryPerformanceCounter(&timerCount);
		
		return (timerCount.QuadPart * 1000.0)/timerFreq.QuadPart;
	}
#else
	double timer()
	{
		struct timeval t;
		gettimeofday(&t, NULL);
		return (t.tv_sec * 1e6 + t.tv_usec)/1000.0;
	}
#endif

