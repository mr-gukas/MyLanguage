#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>


#ifndef ASSERT
#define ASSERT ;
#endif

struct fileLines
{
    char* lineStart = NULL;///< a pointer on the begin of line
    size_t lineLen = 0;///< length of line
    size_t lineIndex = 0;///< index of the line in the source text
};

struct TEXT
{
    char * buf = NULL;///<text
    size_t nChar = 0;///<count of characters
    fileLines * lines= NULL;///<fileLines array
    size_t nLines = 0;///<count of lines
};

void textCtor(TEXT * text, FILE* const source);

size_t fileSize(FILE* const file);

void countInText(char* buf, char search, size_t* count);

void readInBuf(TEXT* text, FILE* source);

void matchLines(TEXT* text);

void textDtor(TEXT* text); 

char* SkipSpaces(char** src, size_t* lineIndex);
