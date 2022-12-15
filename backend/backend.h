#pragma once

#include "stack/stack.h"
#include "../tree/tree.h"
#include "../parser.h"

#define UNDEF_VAR(node) \
	fprintf(stderr, "An uninitialized variable was encountered: <%s>\n", node->name); \
	abort();

#define UNDEF_FUNC(node) \
	fprintf(stderr, "An unknown function encountered: <%s>\n", node->name); \
	abort();


#define ALREADY_DEF_VAR(node) \
	fprintf(stderr, "The variable has already been declared: <%s>\n", node->name); \
	abort();


const size_t VAR_COUNT  = 10000;
const size_t FUNC_COUNT = 10000;
const size_t POISON_PTR = 31415;

struct Variable_t
{
	char*  name;
	size_t pos;
};

struct VarTable_t
{
	Variable_t* arr;
	size_t     size;

	size_t raxPos; 
};

struct Func_t
{
	char*  name;
	size_t  parCount;
};

struct Program_t
{
	TreeNode_t*			 node;
	struct Stack_t*    tabStk;
	size_t		    ifCounter;
	size_t       whileCounter;
	Func_t*		   	  funcArr;
	size_t          funcCount;

	FILE* asmFile;
};

enum ProgramStatus
{
	OK		   = 0,
	WRONG_DATA,
	NULL_TREE,
	UNDEF_VAR,
	ALREADY_DEF_VAR,
	ALREADY_DEF_FUNC,
	UNDEF_FUNC,
	NO_MEM,
	BAD_CALL,
};

int WriteAsmCode(TreeNode_t* root);
int ProgramCtor(Program_t* program, TreeNode_t* node);
int GetVarPos(TreeNode_t* node, Stack_t* stk);
 VarTable_t* GetTableFromStk(Stack_t* stk, size_t index);
 int CheckForVar(TreeNode_t* node, Stack_t* stk);
 void CountExpression(TreeNode_t* node, Program_t* program);
 int ParseRet(TreeNode_t* node, Program_t* program);
 int ParseWhile(TreeNode_t* node, Program_t* program);
 int ParseIf(TreeNode_t* node, Program_t* program);
 int ParseAssign(TreeNode_t* node, Program_t* program);
 int ParseDef(TreeNode_t* node, Program_t* program);
 size_t ParseParams(TreeNode_t* node, VarTable_t* table);
 VarTable_t* VarTableCtor(Stack_t* stk);
 int ParseCall(TreeNode_t* node, Program_t* program);
 int ParseFunc(TreeNode_t* node, Program_t* program);
void ParseStatement(TreeNode_t* node, Program_t* program);
 int VarTableDtor(VarTable_t* table);
 Func_t* FuncArrCtor();
 int FuncArrDtor(Func_t* funcArr);
int TreetoAsm(Program_t* program);
size_t CountParams(TreeNode_t* node);
int ParseScanf(TreeNode_t* node, Program_t* program);
int ParsePrintf(TreeNode_t* node, Program_t* program);

