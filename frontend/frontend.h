#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "../filework/filework.h"
#include "../tree/tree.h"
#include "../utils/dsl.h"
#include "../standart/standart.h"

#define STR_EQ(str1, str2) (strcmp (str1, str2) == 0)

#define PRINT_NAME printf("<%s>\n", (*(tokens->firstTok + *curIndex))->name);
#define PRINT_NUM  printf("<%lg>\n", (*(tokens->firstTok + *curIndex))->numVal);

#define SYNTAXCTRL(str) \
		if (!STR_EQ(str, (*(tokens->firstTok + *curIndex))->name)) \
		{ \
			fprintf(stderr, "file: code.txt: Syntax error! Expected: " str " in %lu\n", (*(tokens->firstTok + *curIndex))->lineIndex); \
			abort(); \
		} \
		if (*curIndex+1 < tokens->nodeCount) (*curIndex)++;

struct Tokens_t	
{
	TreeNode_t** firstTok;
	size_t nodeCount;

	char** globVar;
	char** funcVar;
	char** localVar;
};

enum VarDeep
{
	GLOBAL = 0,
	FUNC, 
	LOCAL,
};

struct MemDefender_t
{
	TreeNode_t** adr;
	size_t		size;
};

 void GetTokens(Tokens_t* tokens, FILE* code);
 size_t GetWord(char *dest, const char *src);
 NodeType GetType(const char* word);
 TreeNode_t* GetCode(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef);
 TreeNode_t* GetFunc(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef);
 TreeNode_t* GetFuncDef(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef);
 TreeNode_t* GetArg(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef);
 TreeNode_t* GetAssign(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef);
 TreeNode_t* GetE(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef);
 TreeNode_t* GetT(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef);
 TreeNode_t* GetPow(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef);
 TreeNode_t* GetP(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef);
 TreeNode_t* GetCall(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef);
 TreeNode_t* GetCond(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef);
 TreeNode_t* GetStat(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef);
 TreeNode_t* GetN(Tokens_t* tokens, size_t* curIndex);
 OperationType GetOpType(const char* name);
 void TokensDtor(Tokens_t* tokens, MemDefender_t* memdef);
 void MemDefenderCtor(MemDefender_t* memdef, size_t size);
 TreeNode_t* GetDef(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef);
 TreeNode_t* GetRet(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef);
 TreeNode_t* GetIf(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef);
 TreeNode_t* GetWhile(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef);
 char* SkipComments(char** src);

 TreeNode_t* GetPrintf(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef);
 TreeNode_t* GetScanf(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef);
