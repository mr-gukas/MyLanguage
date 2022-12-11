#include "backend.h"

int WriteAsmCode(TreeNode_t* root)
{
	//fprintf(asmCode, "CALL :main\n");
	//fprintf(asmCode, "HLT\n");
	
	Program_t program = {};
	ProgramCtor(&program, root);
	
	TreetoAsm(&program);

	return 0;
}

int ProgramCtor(Program_t* program, TreeNode_t* node)
{
	if (program == NULL) return WRONG_DATA;
	if (node    == NULL) return NULL_TREE;
	
	program->node         = node;
	program->ifCounter    = 0;
	program->whileCounter = 0;
	
	Stack_t tabStk = {};
	StackCtor(&tabStk, 10);
	
	FILE* asmCode    = fopen("obj/arm.asm", "w+");
	program->asmFile = asmCode;

	return OK;
}

int TreetoAsm(Program_t* program)
{
	if (program == NULL) return WRONG_DATA;
	
	TreeNode_t* curNode = program->node;

	if (curNode->left->type != Type_FUNC)
	{
		size_t glCounter = 0;
		VarsCount(curNode, &glCounter);
		
		VarTable_t globVars = {};
		CreateVarTable(&globVars, glCounter);
		FillVarTable(&globVars, curNode);
	}

	if (program->node->left->type = Type_DEF)
	{
		TreeNode_t* curNode = program->node;
		size_t	  glCounter = 0;
		while (curNode->left->type == Type_DEF)
		{
			glCounter++;
			curNode = curNode->right;
		}
	
		Variable_t* globVar = CreateVarTable(glCounter);
		FillVarTable(globVar, node, progtam->tabStk); 
	}
		
	return OK;
}

static void CreateVarTable(VarTable_t* table, size_t size)
{
	table->arr = (Variable_t*) calloc(size, sizeof(Variable_t));
	return table;
}

void ParseStatement(TreeNode_t* node, Program_t* program)
{
	assert(node->type == Type_STAT);

	VarTable_t* newTable = NULL;

	if (node->parent && node->parent != Type_FUNC)
		newTable = VarTableCtor(program->tabStk);

	TreeNode_t* curNode = node;
	
	while (curNode)
	{
		switch(node->left)
		{
			case Type_DEF:    ParseDef   (node->left, program); break;
			case Type_ASSIGN: ParseAssign(node->left, program); break;
			case Type_FUNC:	  ParseFunc  (node->left, program); break;
			case Type_CALL:   ParseCall  (node->left, program); break;
			case Type_IF:     ParseIf    (node->left, program); break;
			case Type_WHILE:  ParseWhile (node->left, program); break;
			case Type_RETURN: ParseRet   (node->left, program); break;

			default: abort(); 
		}

		curNode = curNode->right;
	}
}

static int ParseFunc(TreeNode_t* node, Program_t* program)
{
	fprintf(program->asmFile, "%s:\n", node->left->name);

	VarTable_t* newTable = VarTableCtor(program->tabStk);

	if (node->left->left)
		ParseParams(node->left->left, newTable);
		
	ParseStatement(node->right, program);
	
	VarTableDtor(newTable);
	StackPop(program->stk);			
	
	VarTable_t* curTable = GetTableFromStk(stk, stk->size - 1);
	fprintf(program->asmFile, "PUSH %lu\n", curTable->raxPos);
	fprintf(program->asmFile, "POP RAX\n");	

	return OK;
}

static VarTable_t* VarTableCtor(Stack_t* stk)
{
	VarTable_t* newTable = (VarTable_t*) calloc(1, sizeof(VarTable_t));

	if (stk->size >= 1)
	{
		VarTable_t* curTable = 	GetTableFromStk(stk, stk->size - 1);
		newTable->raxPos = curTable->raxPos + curTable->size; 
	}
	else
		newTable->raxPos = 0;
	
	newTable->arr = (Variable_t*) calloc(VAR_COUNT, sizeof(Variable_t));
	StackPush(stk, newTable);

	return newTable;
}

static int ParseParams(TreeNode_t* node, VarTable_t* table)
{
	TreeNode_t* curNode = node;
	
	while (curNode)
	{
		Variable_t param = {};
		param->name      = curNode->left->name;
		param->pos       = table->size;
		table->arr[size] = param;

		table->size++;
		curNode = curNode->right;
	}

	return OK;
}

static int ParseDef(TreeNode_t* node, Program_t* program)
{
	if (!CheckForVars(node->right, program->tabStk))
	{
		return UNDEF_VAR;
	}

	if (CheckForVar(node->left, program->tabStk))
	{
		return ALREADY_DEF_VAR;
	}
	
	Variable_t* newVar   = (Variable_t*) calloc(1, sizeof(Variable_t));
	VarTable_t* curTable = GetTableFromStk(program->tabStk,  program->tabStk->size - 1);

	newVar->name       = node->left->name;
	newVar->pos        = curTable->size;
		
	curTable->arr[curTable->size] = newVar;
	curTable->size += 1;
	
	CountExpression(node->right, program);
	fprintf(program->asmFile, "POP [%lu+RAX]\n", newVar->pos);
	
	return OK;
}

static int ParseAssign(TreeNode_t* node, Program_t* program)
{
	if (!CheckForVars(node->right, program->tabStk))
	{
		return UNDEF_VAR;
	}

	if (!CheckForVar(node->left, program->tabStk))
	{
		return UNDEF_VAR;
	}
	
	size_t pos = GetVarPos(node->left, program->tabStk);
	CountExperssion(node->right, program);
	
	fprintf(program->asmFile, "POP [%lu+RAX]\n", pos);
	
	return OK;
}

static int ParseIf(TreeNode_t* node, Program_t* program)
{
	if (!CheckForVar(node->left, program->tabStk))
	{
		return UNDEF_VAR;
	}
	
	CountExpression(node->left, program);
	fprintf(program->asmFile, "PUSH 0\n");

	if (node->right->type != Type_ELSE)
	{
		fprintf(program->asmFile, "JNE :if_node_%lu\n", program->ifCounter);

		ParseStatement(node->right, program);

		VarTable_t* curTable = GetTableFromStk(stk, stk->size - 1);
		VarTableDtor(curTable)
		StackPop(program->stk);			
		
		curTable = GetTableFromStk(stk, stk->size - 1);
		fprintf(program->asmFile, "PUSH %lu\n", curTable->raxPos);
		fprintf(program->asmFile, "POP RAX\n");	

		fprintf(program->asmFile, "if_%lu:\n", program->ifCounter++);
	}
	else
	{
		fprintf(program->asmFile, "JNE :else_%lu\n", program->ifCounter);

		ParseStatement(node->right->left, program);

		VarTable_t* curTable = GetTableFromStk(stk, stk->size - 1);
		VarTableDtor(curTable)
		StackPop(program->stk);			
		
		curTable = GetTableFromStk(stk, stk->size - 1);
		fprintf(program->asmFile, "PUSH %lu\n", curTable->raxPos);
		fprintf(program->asmFile, "POP RAX\n");	
		
		fprintf(program->asmFile, "JMP :if_%lu\n", program->ifCounter);
		fprintf(program->asmFile, "else_%lu:\n", program->ifCounter);

		ParseStatement(node->right->right, program);
		
		curTable = GetTableFromStk(stk, stk->size - 1);
		VarTableDtor(curTable)
		StackPop(program->stk);			
		
		curTable = GetTableFromStk(stk, stk->size - 1);
		fprintf(program->asmFile, "PUSH %lu\n", curTable->raxPos);
		fprintf(program->asmFile, "POP RAX\n");	
		
		fprintf(program->asmFile, "if_%lu\n", program->ifCounter++);	
	}
	
	return OK;
}

static int ParseWhile(TreeNode_t* node, Program_t* program)
{
	if (!CheckForVar(node->left, program->tabStk))
	{
		return UNDEF_VAR;
	}
	
	fprintf(program->asmFile, "while_%lu:\n", program->whileCounter);

	CountExpression(node->left, program);
	fprintf(program->asmFile, "PUSH 0\n");

	fprintf(program->asmFile, "JNE :while_no_%lu\n", program->whileCounter);

	ParseStatement(node->right, program);
	
	VarTable_t* curTable = GetTableFromStk(stk, stk->size - 1);
	VarTableDtor(curTable)
	StackPop(program->stk);			
	
	curTable = GetTableFromStk(stk, stk->size - 1);
	fprintf(program->asmFile, "PUSH %lu\n", curTable->raxPos);
	fprintf(program->asmFile, "POP RAX\n");	
	
	fprintf(program->asmFile, "JMP while_%lu:\n", program->whileCounter++);	
	fprintf(program->asmFile, "while_no_%lu:\n",  program->whileCounter++);	
	
	return OK;
}

static int ParseRet(TreeNode_t* node, Program_t* program)
{
	if (!CheckForVar(node->left, program->tabStk))
	{
		return UNDEF_VAR;
	}	

	CountExpression(node->left,  program);
	fprintf(program->asmFile, "POP RCX\n"
							  "RET\n");
}

static void CountExpression(TreeNode_t* node, Program_t* program)
{
	if (node->left)  CountExpression(node->left, program);
	if (node->right) CountExpression(node->right, program);
	
	if (node->type == Type_NUM)
	{
		fprintf(program->asmFile, "PUSH %lg\n", node->numVal);
	}
	else if (node->type == Type_VAR)
	{
		size_t varPos = GetVarPos(node, program->tabStk);

		fprintf(program->asmFile, "PUSH [%lu+RAX]\n", varPos);
	}
	else if (node->type == Type_CALL)
	{
		fprint(program->asmFile, "hello world\n");
	}
	
	if (node->type == Type_OP)
	{
		switch (node->opVal)
		{
			case Op_Add: fprint(program->asmFile, "ADD\n"); break;
			case Op_Sub: fprint(program->asmFile, "SUB\n"); break;
			case Op_Mul: fprint(program->asmFile, "MUL\n"); break;
			case Op_Div: fprint(program->asmFile, "DIV\n"); break;
			case Op_Pow: fprint(program->asmFile, "POW\n"); break;
			case Op_Sin: fprint(program->asmFile, "ADD\n"); break;
			case Op_Cos: fprint(program->asmFile, "COS\n"); break;
			case Op_Ln:  fprint(program->asmFile, "LN\n"); break;
			
			default: break;
		}
	}
}


static int CheckForVar(TreeNode_t* node, Stack_t* stk)
{
	if (node->left)
	{
		(!CheckForVar(node->left, stk)) return 0;
	}
	
	if (node->right)
	{
		if (!CheckForVar(node->right, stk)) return 0;
	}

	size_t stkIndex      = stk->size - 1;
	VarTable_t* curTable = GetTableFromStk(stk, stkIndex);
	
	if (node->type == Type_VAR)
	{
		while (stkIndex >= 0)
		{
			for (size_t varIndex = 0; varIndex < curTable->size; varIndex++)
			{
				if (STR_EQ(node->name, (curTable->arr[varIndex])->name))
					return 1; //if we met the variable
			}

			stkIndex--;
			curTable = GetTableFromStk(stk, stkIndex);
		}

		return 0; //if variable is not defined
	}

	return 1;
	
}
				
static VarTable_t* GetTableFromStk(Stack_t* stk, size_t index)
{
	return (stk->data)[index];
}


static size_t GetVarPos(TreeNode_t* node, Stack_t* stk)
{
	size_t stkIndex      = stk->size - 1;
	VarTable_t* curTable = GetTableFromStk(stk, stkIndex);
	
	while (stkIndex >= 0)
	{
		for (size_t varIndex = 0; varIndex < curTable->size; varIndex++)
		{
			if (STR_EQ(node->name, (curTable->arr[varIndex])->name))
				return (curTable->arr[varIndex])->pos + curTable->raxPos;
		}

		stkIndex--;
		curTable = GetTableFromStk(stk, stkIndex);
	}

	return POISON_PTR;
}


