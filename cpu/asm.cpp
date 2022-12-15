#include "cpu.h"

int main(int argc, const char* argv[])
{
    FILE* source    = NULL;
    FILE* processed = NULL;

    if (argc != 3)
    {
        fprintf(stderr, ">>>Incorrect command line arguments. Exiting from programm....\n");
        abort(); 
    }

    if ((source = fopen(argv[1], "r")) == NULL)
    {
        fprintf(stderr, ">>>Problems with file with source commands: %s\n", argv[1]);
        abort();
    }

    processed = fopen(argv[2], "w+");
    
    AsmCmd_t sourceCmd = {};

    $$(asmCtor(&sourceCmd, source));
    RunAsm(&sourceCmd);
    $$(FillBin(&sourceCmd, processed));
    $$(asmDtor(&sourceCmd));

    fclose(source);
    fclose(processed);

}

void RunAsm(AsmCmd_t* asmCmd)
{
    ASSERT(asmCmd != NULL);

    $$(asmMakeArr(asmCmd));
    $$(asmMakeArr(asmCmd));
}

int asmCtor(AsmCmd_t* asmCmd, FILE* source)
{
    ASSERT(asmCmd != NULL);
    ASSERT(source != NULL);
    
    textCtor(&asmCmd->commands, source);

    asmCmd->listfile   = fopen(LISTING_FILE, "w+");
    asmCmd->info.sign  = BASIC_SIGN;
    asmCmd->info.vers  = BASIC_VERS;
    asmCmd->info.nCmd  = asmCmd->commands.nLines - 1;
    asmCmd->asmArr     = (unsigned char*) calloc(asmCmd->info.nCmd * 2, sizeof(arg_t));
    
    for (size_t index = 0; index < MAX_LABEL_COUNT; ++index)
    {
        asmCmd->labels[index].address = POISON_ARG; 
        strncpy(asmCmd->labels[index].name, POISON_NAME, LABEL_SIZE);
    }

    ASSERT(asmCmd->asmArr != NULL);

    return 0;
}

int asmDtor(AsmCmd_t* asmCmd)
{
    ASSERT(asmCmd != NULL);
    
    fclose(asmCmd->listfile);
    textDtor(&asmCmd->commands);
    free(asmCmd->asmArr);

    return 0;
}

int asmMakeArr(AsmCmd_t* asmCmd)
{
    ASSERT(asmCmd != NULL);

    size_t index = 0;
    size_t ip    = 0;
    
    fprintf(asmCmd->listfile, "\n\n---------------------COMMAND LIST---------------------\n");

    for (index = 0, ip = 0; index < asmCmd->info.nCmd; ++index)
    {
        char cmd[STR_MAX_SIZE] = {};
        
        fprintf(asmCmd->listfile, "\n%04X  |  %-15s  |  ",  ip, asmCmd->commands.lines[index].lineStart);
        
        if (asmCmd->commands.lines[index].lineStart[0] == '/' || asmCmd->commands.lines[index].lineStart[0] == '\0')
        {
            continue;
        }

        int nChar = 0;
        sscanf(asmCmd->commands.lines[index].lineStart, "%s%n", cmd, &nChar);
        
        if (nChar <= 0)
        {
            return 1; 
        }

#define DEF_CMD(name, num, arg, code)                                                       \
    if (strcasecmp(cmd, #name) == 0)                                                         \
    {                                                                                         \
        if (arg)                                                                               \
        {                                                                                       \
           $$(MakeArg(asmCmd->commands.lines[index].lineStart + nChar, CMD_##name, asmCmd, &ip));\
        }                                                                                         \
        else                                                                                       \
        {                                                                                           \
            *(asmCmd->asmArr + ip++) = CMD_##name;                                                   \
            fprintf(asmCmd->listfile, "%02X ", *(asmCmd->asmArr + ip - 1));                           \
        }                                                                                              \
    }                                                                                                   \
    else

#define DEF_JMP(name, num, sign)                                                           \
    if (strcasecmp(cmd, #name) == 0)                                                        \
    {                                                                                        \
        $$(MakeArg(asmCmd->commands.lines[index].lineStart + nChar, JMP_##name, asmCmd, &ip));\
    }                                                                                          \
    else

#include "cmd.h"

        /*else*/ if (strchr(cmd, ':') != NULL)
        {   
            LabelAnalyze(cmd, asmCmd, ip); 
        }
    
        else 
        {
            return 1;
        }
    }
    
#undef DEF_CMD
#undef DEF_JMP
    
    asmCmd->info.filesize = ip - 1;

    return 0;
}

int FillBin(AsmCmd_t* asmCmd, FILE* binary)
{
    ASSERT(asmCmd != NULL);
    ASSERT(binary != NULL);

    fwrite(&asmCmd->info,  sizeof asmCmd->info, 1, binary);
    fwrite(asmCmd->asmArr, sizeof (unsigned char), asmCmd->info.filesize + 1, binary);

    return 0;
}

void CopyVal(unsigned char* arr, const arg_t* value) 
{   
    ASSERT(arr   != NULL);
    ASSERT(value != NULL);
   
    memcpy(arr, value, sizeof (arg_t));  
}

int IsRegister(const char* reg)
{
    ASSERT(reg != NULL);

    if (strlen(reg) == 3 && reg[0] == 'R' && reg[2] == 'X' && reg[1] >= 'A' && reg[1] <= 'I')
        return reg[1] - 'A' + 1;

    return -1;
}

int MakeArg(char* line, int command, AsmCmd_t* asmCmd, size_t* ip)
{
    ASSERT(line   != NULL);
    ASSERT(asmCmd != NULL);
    ASSERT(ip     != NULL);
    
    int argCtrl = 0; 

    if (command >= JMP_JMP && command <= JMP_CALL)
    {
        argCtrl = MakeJumpArg(line, command, asmCmd, ip);
    }
    
    else
    {
        switch (isInBrackets(line))
        {
            case 0:
                argCtrl = MakeCommonArg(line, command, asmCmd, ip);
                break;
            case 1:
                argCtrl = MakeBracketsArg(line, command, asmCmd, ip);
                break;
            case -1:
            default:
                argCtrl = 1;
        }
    }

    if (argCtrl)
    {
        return 1;
    }

    return 0;
}

int isInBrackets(char* arg)
{
    ASSERT(arg != NULL);
    
    char* open  = strchr(arg, '[');
    char* close = strchr(arg, ']');

    if (open == NULL && close == NULL)
        return 0;
    
    if ((open && !close) || (!open && close))
        return -1;
    
    return 1;
}

int MakeCommonArg(char* line, int command, AsmCmd_t* asmCmd, size_t* ip)
{
    ASSERT(line   != NULL);
    ASSERT(asmCmd != NULL);
    ASSERT(ip     != NULL);

    arg_t  curValue              = 0;
    char   curReg[STR_MAX_SIZE]  = {};
    int    intReg                = 0;
    
    if (strchr(line, '+') == NULL)
    {
        if (command != CMD_POP && sscanf(line, TYPE_ARG_FMT, &curValue) == 1)
        {
            *(asmCmd->asmArr + *ip) = CMD_PUSH | ARG_IMMED; 
            CopyVal(asmCmd->asmArr + *ip + 1, &curValue);

            fprintf(asmCmd->listfile, "%02X " TYPE_ARG_FMT,  *(asmCmd->asmArr + *ip),  curValue);
			
			*ip += 1 + sizeof(arg_t);
        }

        else if (sscanf(line, "%19s", curReg) == 1  && (intReg = IsRegister(curReg)) != -1)
        {
            *(asmCmd->asmArr + *ip) = command | ARG_REG;
			memcpy(asmCmd->asmArr + *ip + 1, &intReg, sizeof (int));  

            fprintf(asmCmd->listfile, "%02X %02X",  *(asmCmd->asmArr + *ip), intReg);


			*ip += 1 + sizeof(int);
        }
        
        else
            return 1;
        
    }
    
    else
    {
        if (command != CMD_POP && sscanf(line, TYPE_ARG_FMT "+%19s", &curValue, curReg) == 2 &&
                                        (intReg = IsRegister(curReg)) != -1)
        {
            *(asmCmd->asmArr + *ip) = CMD_PUSH | ARG_IMMED | ARG_REG;  
            
			memcpy(asmCmd->asmArr + *ip + 1, &curValue, sizeof (arg_t));  
			memcpy(asmCmd->asmArr + *ip + 1 + sizeof(arg_t), &intReg, sizeof (int));  

            fprintf(asmCmd->listfile, "%02X %lg %02X ",  *(asmCmd->asmArr + *ip),  curValue, intReg);
        }

        else
            return 1;

        *ip += sizeof(arg_t) + sizeof(int) + 1;
    }

    return 0;
}

int MakeBracketsArg(char* line, int command, AsmCmd_t* asmCmd, size_t* ip)
{
    ASSERT(line   != NULL);
    ASSERT(asmCmd != NULL);
    ASSERT(ip     != NULL);
    

    char* arg = strchr(line, '[') + 1;
    
    int  curValue              = 0;
    char curReg[STR_MAX_SIZE]  = {};
    int  intReg                = 0;
    
    if (strchr(line, '+') == NULL)
    {
        if (sscanf(arg, "%d", &curValue) == 1)
        {
            *(asmCmd->asmArr + *ip) = command | ARG_IMMED | ARG_MEM; 
			memcpy(asmCmd->asmArr + *ip + 1, &curValue, sizeof (int));  

            fprintf(asmCmd->listfile, "%02X %02X ",  *(asmCmd->asmArr + *ip),  curValue);
        }
        
        else if (sscanf(arg, "%s", curReg) == 1)
        {
            curReg[strlen(curReg) - 1] = '\0';

            if ((intReg = IsRegister(curReg)) != -1)
            {
                *(asmCmd->asmArr + *ip) = command | ARG_REG | ARG_MEM;
				memcpy(asmCmd->asmArr + *ip + 1, &intReg, sizeof (int));  

                fprintf(asmCmd->listfile, "%02X %02X ",  *(asmCmd->asmArr + *ip),  intReg);
            }
        
            else
			{
                return 1;
			}
        }

        else
		{
            return 1;
		}

        *ip += 1 + sizeof(int);
    }
    
    else
    {
        if (sscanf(arg, "%d+%s", &curValue, curReg) == 2)
        {
			
            curReg[strlen(curReg) - 1] = '\0';
            if ((intReg = IsRegister(curReg)) != -1)
            {
                *(asmCmd->asmArr + *ip) = command | ARG_IMMED | ARG_REG | ARG_MEM;  
				
				memcpy(asmCmd->asmArr + *ip + 1, &curValue, sizeof(int));
				memcpy(asmCmd->asmArr + *ip + 1 + sizeof(int), &intReg, sizeof(int));

                fprintf(asmCmd->listfile, "%02X %02X %02X ",  *(asmCmd->asmArr + *ip),  curValue,  intReg);
            }

            else
                return 1;
        }

        else
            return 1;

        *ip += 2 * sizeof(int) + 1;
    }

    return 0;
}

int LabelAnalyze(char* cmd, AsmCmd_t* asmCmd, size_t ip)
{
    ASSERT(cmd    != NULL);
    ASSERT(asmCmd != NULL);

    int  curLabel                 = -1;
    char curTextLabel[LABEL_SIZE] = {};

    if (sscanf(cmd, "%d:", &curLabel) == 1)
    {
        if (curLabel >= 0 && curLabel < MAX_LABEL_COUNT && asmCmd->labels[curLabel].address == POISON_ARG)
        {
            asmCmd->labels[curLabel].address = ip;
            
        }
        else if (!(curLabel >= 0 && curLabel < MAX_LABEL_COUNT && asmCmd->labels[curLabel].address != POISON_ARG))
        {
            return 1;
        }
    }
    else if (sscanf(cmd, "%19s", curTextLabel) == 1) 
    {
        int labelCtrl = 1;
        
        for (size_t num = 0; num < MAX_LABEL_COUNT; ++num)
        {
            char* dbDot = strchr(curTextLabel, ':');
            if (dbDot) *dbDot = '\0';

            if (strcmp(asmCmd->labels[num].name, POISON_NAME) == 0)
            {
                strncpy(asmCmd->labels[num].name, curTextLabel, LABEL_SIZE);
                asmCmd->labels[num].address = ip;
                curLabel                   = num;    

                labelCtrl = 0;
                break;
            }

            else if (strcmp(asmCmd->labels[num].name, curTextLabel) == 0)
            {
                labelCtrl = 0;
                break;
            }

        }
        
        return labelCtrl;
    }
    
    return 0;
}

int MakeJumpArg(char* line, int command, AsmCmd_t* asmCmd, size_t *ip)
{
    ASSERT(line   != NULL);
    ASSERT(asmCmd != NULL);
    ASSERT(ip     != NULL);
    
    *(asmCmd->asmArr + *ip) = command;
	
    int  curValue                 = 0;
    char curTextLabel[LABEL_SIZE] = {};

    if (strchr(line, ':') == NULL)
    {   
        if (sscanf(line, "%d", &curValue) == 1)
        {
			memcpy(asmCmd->asmArr + *ip + 1, &curValue, sizeof(int));
        }

        else
            return 1;
    }
    
    else
    {   
        if (sscanf(line, " :%d", &curValue) == 1 && curValue >= 0 && curValue <= MAX_LABEL_COUNT)
        {  

            if (asmCmd->labels[curValue].address  == POISON_ARG)
            {   
                curValue = POISON_ARG;
				memcpy(asmCmd->asmArr + *ip + 1, &curValue, sizeof(int));
            }
            
            else
            {
				memcpy(asmCmd->asmArr + *ip + 1, &asmCmd->labels[curValue].address, sizeof(int));
            }
        }
        
        else if (sscanf(line, " :%s", curTextLabel) == 1)
        {
            for (size_t num = 0; num < MAX_LABEL_COUNT; ++num)
            {
                if (strcmp(asmCmd->labels[num].name, curTextLabel) == 0)
                {
					memcpy(asmCmd->asmArr + *ip + 1, &asmCmd->labels[num].address, sizeof(int));
                    curValue = num;

                    break;
                }
            }
            
        }

    }

    fprintf(asmCmd->listfile, "%02X %02X ",  *(asmCmd->asmArr + *ip),  asmCmd->labels[curValue].address);


    *ip += sizeof(int) +  1;

    return 0;
}

