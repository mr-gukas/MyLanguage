#pragma once

#include "stack/stack.h"
#include "../tree/tree.h"

const size_t VAR_COUNT  = 10000;
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
	VarTable_t* params;
};

struct Program_t
{
	TreeNode_t*			 node;
	struct Stack_t*    tabStk;
	size_t		    ifCounter;
	size_t       whileCounter;
	Func_t*		   	  funcArr;

	FILE* asmFile;
};

enum ProgramStatus
{
	OK		   = 0,
	WRONG_DATA,
	NULL_TREE,
	UNDEF_VAR,
	ALREADY_DEF_VAR,
	UNDEF_FUNC,
	NO_MEM,
};

int WriteAsmCode(TreeNode_t* root);
int ProgramCtor(Program_t* program, TreeNode_t* node);

