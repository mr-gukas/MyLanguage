#define POP           StackPop(&cpu->stk)
#define POP_RET       StackPop(&cpu->retStk)
#define PUSH(arg)     StackPush(&cpu->stk, (arg_t) (arg))
#define PUSH_RET(arg) StackPush(&cpu->retStk, (arg))
#define VAR           arg_t
#define GET_PUSH_ARG  GetPushArg(curCmd, &ip, cpu) 
#define GET_POP_ARG   GetPopArg(curCmd, &ip, cpu)
#define GET_JMP_ARG   GetJumpArg(&ip, cpu)
#define DUMP_CPU      CpuDump(cpu, ip);
#define INDEX_UP      ++ip
#define BAD_DIV                                          \
    {                                                     \
        fprintf(stderr, "CANNOT BE DIVIDED BY ZERO!!!\n"); \
        abort();                                            \
    }

#define BAD_INP                                   \
    {                                              \
        fprintf(stderr, "Incorrect input value\n"); \
        abort();                                     \
    }

#define SHOW_RAM                                      \
    system("clear");                                   \
    printf("- - - - - - - - - - - - - - - - - \n");     \
    for (size_t index = 0; index < MAX_RAM_SIZE; ++index)\
    {                                                     \
        if (index % 15 == 0) printf("|");                  \
        printf((cpu->RAM[index]) ? "* " : "  ");            \
        if ((index + 1) % 15 == 0)                           \
        printf("|\n");                                        \
    }                                                          \
    printf("- - - - - - - - - - - - - - - - - \n");             \

