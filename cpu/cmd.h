#include "dsl.h"

DEF_CMD(HLT,  0, 0, 
{
    ip = cpu->size + 7;
})

DEF_CMD(PUSH, 1, 1, 
{
    INDEX_UP;
    VAR ARG = GET_PUSH_ARG; 

    PUSH(ARG);
})

DEF_CMD(ADD,  2, 0, 
{
    VAR NUM2 = POP;
    VAR NUM1 = POP;

    PUSH((NUM1 + NUM2));
})

DEF_CMD(SUB,  3, 0,
{
    VAR NUM2 = POP;
    VAR NUM1 = POP;
    PUSH((NUM1 - NUM2));
})

DEF_CMD(MUL,  4, 0,
{
    VAR NUM2 = POP;
    VAR NUM1 = POP;

    PUSH((NUM2 * NUM1));
})

DEF_CMD(DIV,  5, 0,
{
    VAR NUM2 = POP;
    VAR NUM1 = POP;
    
    if (NUM2 == 0)
        BAD_DIV;
    
    PUSH((NUM1 / NUM2));
})

DEF_CMD(OUT,  6, 0,
{
    printf("%lg\n", POP);
})

DEF_CMD(INP,  7, 0,
{
    VAR NUM  = 0;

    if (scanf("%lf", &NUM) != 1)
        BAD_INP;

    PUSH(NUM);
})

DEF_CMD(DUMP, 8, 0,
{
    DUMP_CPU;
})

DEF_CMD(POP, 9, 1,
{
    INDEX_UP;
    VAR* ARG = GET_POP_ARG;
    *ARG = POP;
})

DEF_CMD(INF, 22, 0,
{
    printf("infinite number of roots\n");
})

DEF_CMD(NO_SOL, 18, 0,
{
    printf("no solutions\n");
})

DEF_CMD(ROOT_PR, 19, 0,
{
    VAR INTEG = POP;
    VAR FRAC  = POP;
    
    printf("%d.%d\n", INTEG, FRAC);
})

DEF_CMD(POW, 20, 0, 
{
	VAR	DEG = POP;
    VAR ARG = POP;
    PUSH(pow(ARG, DEG));
})

DEF_CMD(MINUS_PR, 21, 0,
{
    printf("-");
})

DEF_CMD(RET, 23, 0, 
{
    size_t ret_ip = (size_t) POP_RET;
    ip = ret_ip;
})

DEF_JMP(JMP, 10, || 1 ||)

DEF_JMP(JA,  11,       >)

DEF_JMP(JAE, 12,      >=)

DEF_JMP(JE,  13,      ==)

DEF_JMP(JBE, 14,      <=)

DEF_JMP(JB,  15,       <)

DEF_JMP(JNE, 16,      !=)

DEF_JMP(CALL, 17, || 1 ||)

