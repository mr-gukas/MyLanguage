#include "filework.h"

void textCtor(TEXT * text, FILE* const source)
{
    ASSERT(source    != NULL);
    ASSERT(text      != NULL);

    readInBuf(text, source);

    matchLines(text);
}

void readInBuf(TEXT* text, FILE* source)
{
    ASSERT(text   != NULL);
    ASSERT(source != NULL);
    
    text->nChar = fileSize(source);
    text->buf   = (char *) calloc(text->nChar + 1, sizeof(char));
    
    fread(text->buf, sizeof(char), text->nChar, source);

}

size_t fileSize(FILE* file)
{
    ASSERT(file != NULL);

	fseek(file, 0, SEEK_END);
    size_t pos = (size_t) ftell(file);
    fseek(file, 0, SEEK_SET);

    return pos;
}

void matchLines(TEXT* text)
{
    size_t   line        = 0;
    size_t   lineStart   = 0;
    
    countInText(text->buf, '\n', &(text->nLines));
    text->lines = (fileLines*) calloc(text->nLines + 1, sizeof(fileLines));
    
    for (size_t index = 1; index < text->nChar; index++)
    {
        if (*(text->buf + index) == '\n')
        {
            (*(text->buf + index)= '\0');

            (text->lines + line)->lineStart = text->buf + lineStart;
            (text->lines + line)->lineLen   = index - lineStart;
            (text->lines + line)->lineIndex = line;
            lineStart                       = index + 1;
            ++line;
        }

        if (lineStart > text->nChar)
        {
            break;
        }
    }

}
     
void countInText(char* text, char search, size_t* count)
{
    size_t index = 0;

    while (*(text + index))  
    {
        if (*(text + index) == search)
        {
            ++(*count);
        }
        ++index;
    }
}

void textDtor(TEXT* text)
{
    ASSERT(text != NULL);

    free(text->buf); 
    free(text->lines);
}



