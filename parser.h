#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "filework/filework.h"
#include "tree/tree.h"
#include "dsl.h"
#include "backend/backend.h"


#define STR_EQ(str1, str2) (strncmp (str1, str2, strlen (str1)) == 0)

#define PRINT_NAME printf("<%s>\n", (*(tokens->firstTok + *curIndex))->name);
#define PRINT_NUM  printf("<%lg>\n", (*(tokens->firstTok + *curIndex))->numVal);

#define SYNTAXCTRL(str) \
		if (!STR_EQ(str, (*(tokens->firstTok + *curIndex))->name)) \
		{ \
			fprintf(stderr, "%d: Syntax error! Expected: " str " in %lu\n", __LINE__, (*(tokens->firstTok + *curIndex))->lineIndex); \
			abort(); \
		} \
		if (*curIndex+1 < tokens->nodeCount) (*curIndex)++;

const size_t VAR_COUNT = 30;

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

static void GetTokens(Tokens_t* tokens, FILE* code);
static char* SkipSpaces(char** src, size_t* lineIndex);
static size_t GetWord(char *dest, const char *src);
static NodeType GetType(const char* word);
static TreeNode_t* GetCode(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef);
static TreeNode_t* GetFunc(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef);
static TreeNode_t* GetFuncDef(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef);
static TreeNode_t* GetArg(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef);
static TreeNode_t* GetAssign(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef);
static TreeNode_t* GetE(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef);
static TreeNode_t* GetT(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef);
static TreeNode_t* GetPow(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef);
static TreeNode_t* GetP(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef);
static TreeNode_t* GetCall(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef);
static TreeNode_t* GetCond(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef);
static TreeNode_t* GetStat(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef);
static TreeNode_t* GetN(Tokens_t* tokens, size_t* curIndex);
static OperationType GetOpType(const char* name);
static void TokensDtor(Tokens_t* tokens, MemDefender_t* memdef);
static void MemDefenderCtor(MemDefender_t* memdef, size_t size);
static TreeNode_t* GetDef(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef);
static TreeNode_t* GetRet(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef);
static TreeNode_t* GetIf(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef);
static TreeNode_t* GetWhile(Tokens_t* tokens, size_t* curIndex, MemDefender_t* memdef);
static char* SkipComments(char** src);


