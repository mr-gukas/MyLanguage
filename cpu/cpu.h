#include "../filework/filework.h"
#include "stack/stack.h"
#include "../log/LOG.h"
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>


#ifdef LOG_MODE
    FILE* PrintFile = startLog(PrintFile);
#else
    FILE* PrintFile = fopen("cpuLog.txt", "w+");
#endif

typedef double arg_t;

#define TYPE_ARG_FMT "%lg"

#define $$(cmd)                         \
    if (cmd){                            \
        fprintf(stderr, "Error in " #cmd "\n");}
    
const size_t STR_MAX_SIZE    = 20;
const short  BASIC_SIGN      = 'VG';
const size_t BASIC_VERS      = 1;
const size_t REGS_COUNT      = 10;
const size_t MAX_RAM_SIZE    = 225;
const size_t MAX_LABEL_COUNT = 20; 
const size_t LABEL_SIZE      = 20;
const char   LISTING_FILE[]  = "../obj/listing.txt";  
const size_t POISON_ARG      = 314;
char*        POISON_NAME     = "ded32";
const double EPS             = 0.00001;

#define DEF_CMD(name, num, arg, code) \
    CMD_##name = num,

#define DEF_JMP(name, num, sign)      \
    JMP_##name = num,

enum Commands
{
    #include "cmd.h" 
};

#undef DEF_CMD
#undef DEF_JMP

struct Label_t
{
    int   address;
    char  name[LABEL_SIZE];
};

enum Arg_Ctrl
{
    ARG_IMMED = 1 << 5,
    ARG_REG   = 1 << 6,
    ARG_MEM   = 1 << 7,
    ONLY_CMD  = (1 << 5) - 1, 
};

struct CmdInfo_t
{
    short  sign;
    size_t vers;
    size_t nCmd;
    size_t filesize;
};

struct AsmCmd_t
{
    CmdInfo_t          info;
    TEXT               commands;
    unsigned char      *asmArr ; 
    FILE*              listfile;
    Label_t            labels[MAX_LABEL_COUNT];
};

struct Cpu_t
{
    unsigned char* cmdArr;
    size_t         size;
    Stack_t        stk;
    arg_t          regs[REGS_COUNT];
    arg_t          RAM[MAX_RAM_SIZE];
    Stack_t        retStk;
};

int asmCtor(AsmCmd_t* asmCmd, FILE* source);

int asmMakeArr(AsmCmd_t* asmCmd);

int asmDtor(AsmCmd_t* asmCmd);

void RunAsm(AsmCmd_t* asmCmd);

int FillBin(AsmCmd_t* asmCmd, FILE* binary);

unsigned char* ReadBin(unsigned char* cmdArr, FILE* binary);

int RunCpu(Cpu_t* cpu);

int IsRegister(const unsigned char* reg);

void CopyVal(unsigned char* arr, const arg_t* value);

int MakeArg(char* line, int command, AsmCmd_t* asmCmd, size_t *ip);

int CpuCtor(Cpu_t* cpu, FILE* binary);

int CpuDtor(Cpu_t* cpu, FILE* binary);

arg_t GetPushArg(int command, size_t* ip, Cpu_t* cpu);

arg_t* GetPopArg(int command, size_t* ip, Cpu_t* cpu);

int isInBrackets(char* arg);

int MakeCommonArg(char* line, int command, AsmCmd_t* asmCmd, size_t* ip);

int MakeBracketsArg(char* line, int command, AsmCmd_t* asmCmd, size_t* ip);

int MakeJumpArg(char* line, int command, AsmCmd_t* asmCmd, size_t *ip);

int LabelAnalyze(char* cmd, AsmCmd_t* asmCmd, size_t ip);

int GetJumpArg(size_t* ip, Cpu_t* cpu);

int DoubleCmp(double num1, double num2);

void CpuDump(Cpu_t* cpu, size_t ip);
