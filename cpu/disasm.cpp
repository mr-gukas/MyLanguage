#include "disasm.h"

int main(int argc, const char* argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, ">>>Incorrect command line arguments. Exiting the program....\n");
        abort();
    }   

    FILE* binary   = NULL;
    FILE* result   = fopen(argv[2], "w+");
    // can be unable to write file.

    if ((binary = fopen(argv[1], "r")) == NULL)
    {
        fprintf(stderr, ">>>Incorrect filename: %s\n", argv[1]);
        abort(); // abort()
    }
    
    size_t         size   = 0;
    unsigned char* cmdArr = ReadBin(cmdArr, &size, binary);

    // Stack* stk = (Stack*) calloc(1, sizeof(Stack));
    // StackCtor(stk);

    RunDisasm(cmdArr, &size, result);

    free(cmdArr);
}
    
unsigned char* ReadBin(unsigned char* cmdArr, size_t* size, FILE* binary)
{
    ASSERT(binary != NULL);

    CmdInfo_t binInfo = {};

    fread(&binInfo, sizeof(CmdInfo_t), 1, binary);
    if (binInfo.sign != BASIC_SIGN || binInfo.vers != BASIC_VERS)
    {
        fprintf(stderr, ">>>The file extension is incorrect. Exiting the program...\n");
        // where is exiting???
    }
    
    cmdArr = (unsigned char*) calloc (1, binInfo.filesize + 1);
    if (cmdArr == NULL)
    {
        fprintf(stderr, ">>>Problems with allocating memory.\n");
        // where is exiting...
    }

    fread(cmdArr, sizeof(char), binInfo.filesize, binary);
    *size = binInfo.filesize;

    fclose(binary);
   
    return cmdArr;
}

void RunDisasm(unsigned char* cmdArr, size_t* size, FILE* result)
{
    ASSERT(cmdArr != NULL);
    ASSERT(result != NULL);

    size_t ip = 0;
    
    while (ip <= *size)
    {
        int curCmd = *(cmdArr + ip);

#define DEF_CMD(name, num, arg, code)               \
    case CMD_##name:                                \
    {                                               \
        fprintf(result, #name);                     \
        if (arg)                                    \
        {                                           \
            DisasmArg(curCmd, cmdArr, &ip, result); \
        }                                           \
        fprintf(result, "\n");                      \
        break;                                      \
    }

#define DEF_JMP(name, num, sign)                         \
    case JMP_##name:                                     \
    {                                                    \
        fprintf(result, #name);                          \
        arg_t argIm = 0;                                 \
        memcpy(&argIm, cmdArr + ip + 1, sizeof(arg_t));  \
        fprintf(result, " %02X\n", argIm);               \
        ip += sizeof(arg_t);                             \
        break;                                           \
    }

        switch (curCmd & ONLY_CMD)
        {
            #include "cmd.h"

            default:
                fprintf(stderr, ">>>An unknown operation was encountered: %02X\n", *(cmdArr + ip));
        }

        ++ip;
    }

#undef DEF_CMD
#undef DEF_JMP
    
    fclose(result);
}


void DisasmArg(int curCmd, unsigned  char* cmdArr, size_t* ip, FILE* result)
{
    ASSERT(cmdArr != NULL);
    ASSERT(result != NULL);
    ASSERT(ip     != NULL);

    fprintf(result, (curCmd & ARG_MEM) ? " [" : " ");
    
    if ((curCmd & ARG_IMMED) && (curCmd & ARG_REG))
    {
        arg_t argIm = 0;
        arg_t argRe = 0;
        
        memcpy(&argIm, cmdArr + *ip + 1,                 sizeof(arg_t));
        memcpy(&argRe, cmdArr + *ip + 1 + sizeof(arg_t), sizeof(arg_t));

        fprintf(result, TYPE_ARG_FMT "+R%cX", argIm, 'A' + argRe - 1);

        *ip += 2 * sizeof(arg_t);
    }

    else if (curCmd & ARG_IMMED)
    {
        arg_t argIm = 0;
        
        memcpy(&argIm, cmdArr + *ip + 1, sizeof(arg_t));

        fprintf(result, TYPE_ARG_FMT, argIm);

        *ip += sizeof(arg_t);
    }

    else if (curCmd & ARG_REG)
    {
        arg_t argRe = 0;
        
        memcpy(&argRe, cmdArr + *ip + 1, sizeof(arg_t));

        fprintf(result, "R%cX", 'A' + argRe - 1);

        *ip += sizeof(arg_t);
    }

    else
    {
        fprintf(stderr, "BAD ARGUMENT: %04X", curCmd ^ ONLY_CMD);
    }

    fprintf(result, (curCmd & ARG_MEM) ? "]" : "");
}


