#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#ifndef PRINT_FILE
    #define PRINT_FILE "obj/log.htm"
#endif

FILE* startLog(FILE* LogFile);

void endLog(FILE* LogFile);

void printTime(FILE* file);

