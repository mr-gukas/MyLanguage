#include "backend.h"

#ifdef LOG_MODE
    FILE* LogFile = startLog(LogFile);
#endif
int main(void)
{
	Tree_t  stTree = {};

	FILE* standart = fopen("obj/standart.txt", "r");

	assert(standart);

	stTree.root = ReadStandart(standart);
	fclose(standart);
	TreeUpdate(&stTree, stTree.root);
	TreeDump(&stTree);
	WriteAsmCode(stTree.root);

#ifdef LOG_MODE
    endLog(LogFile);
#endif
	return 0;
}

int WriteAsmCode(TreeNode_t* root)
{
	assert(root); 

	Program_t program = {};
	ProgramCtor(&program, root);
	
	TreetoAsm(&program);

	fclose(program.asmFile);

	return 0;
}

int ProgramCtor(Program_t* program, TreeNode_t* node)
{
	if (program == NULL) return WRONG_DATA;
	if (node    == NULL) return NULL_TREE;
	
	program->node         = node;
	program->ifCounter    = 0;
	program->whileCounter = 0;
	
	program->tabStk = (Stack_t*) calloc(1, sizeof(Stack_t));	
	StackCtor(program->tabStk, 10);

	program->funcArr = FuncArrCtor();

	FILE* asmCode    = fopen("obj/arm.asm", "w+");
	program->asmFile = asmCode;

	return OK;
}

int TreetoAsm(Program_t* program)
{
	if (program == NULL) return WRONG_DATA;
	
	ParseStatement(program->node, program);

//TODO 	PROGRAM_DTOR();

	for (size_t index = 0; index < program->tabStk->size; index++)
	{
		VarTableDtor(program->tabStk->data[index]);
	}

	StackDtor(program->tabStk);	
	FuncArrDtor(program->funcArr);
	free(program->tabStk);

	return OK;
}

void ParseStatement(TreeNode_t* node, Program_t* program)
{
	assert(node && program);

	assert(node->type == Type_STAT);

		
	VarTable_t* oldTable = NULL;

	if (program->tabStk->size != 0)
		oldTable = GetTableFromStk(program->tabStk, program->tabStk->size - 1);

	if (program->tabStk->size != 0 && (node->parent && node->parent->type != Type_FUNC))
	{
		fprintf(program->asmFile, "PUSH RAX\n"
								  "PUSH %lu\n"
								  "ADD\n"
								  "POP RAX\n\n", oldTable->size);
	}

	VarTable_t* newTable = NULL;
	
	if (!node->parent || (node->parent && node->parent->type != Type_FUNC))
	{
		newTable = VarTableCtor(program->tabStk);
	}
	
	TreeNode_t* curNode = node;
	int firstFunc       = 0;	
	int isRet           = 1;
	while (curNode)
	{
		if (curNode->left->type == Type_FUNC && !firstFunc)
		{
			fprintf(program->asmFile, "CALL :main\n"
									  "HLT\n");
			firstFunc = 1;
		}

		switch(curNode->left->type)
		{
			case Type_DEF:    ParseDef    (curNode->left, program); break;
			case Type_ASSIGN: ParseAssign (curNode->left, program); break;
			case Type_FUNC:	  ParseFunc   (curNode->left, program); break;
			case Type_PRINTF: ParsePrintf (curNode->left->left, program); break;
			case Type_SCANF:  ParseScanf  (curNode->left->left, program); break;
			case Type_CALL:   ParseCall   (curNode->left, program); break;
			case Type_IF:     ParseIf     (curNode->left, program); break;
			case Type_WHILE:  ParseWhile  (curNode->left, program); break;
			case Type_RETURN: isRet = 1; ParseRet (curNode->left, program); break;

			default: abort(); 
		}
		
		curNode = curNode->right;
	}

	if (!isRet && program->tabStk->size != 0 && (node->parent && node->parent->type != Type_FUNC))
	{
		fprintf(program->asmFile, "PUSH RAX\n"
								  "PUSH %lu\n"
								  "SUB\n"
								  "POP RAX\n\n", oldTable->size);
	}

	
}

int ParseFunc(TreeNode_t* node, Program_t* program)
{
	for (size_t index = 0; index < program->funcCount; index++)
	{
		if (STR_EQ((program->funcArr[index]).name, node->left->name))
		{
			return ALREADY_DEF_FUNC;
		}
	}

	(program->funcArr[program->funcCount]).name = node->left->name;

	if (STR_EQ(node->left->right->name, "krknaki"))
		(program->funcArr[program->funcCount]).retVal = DOUBLE;
	else
		(program->funcArr[program->funcCount]).retVal = VOID;

	
	fprintf(program->asmFile, "\n%s:\n", node->left->name);
	
	VarTable_t* newTable = VarTableCtor(program->tabStk);
	
	if (node->left->left)
		(program->funcArr[program->funcCount]).parCount = ParseParams(node->left->left, newTable);
	

	program->funcCount++;
	ParseStatement(node->right, program);

	VarTableDtor(newTable);
	StackPop(program->tabStk);			
	
	return OK;
}

int VarTableDtor(VarTable_t* table)
{
	free(table->arr);
	free(table);

	return OK;
}

Func_t* FuncArrCtor()
{
	Func_t* funcArr = (Func_t*) calloc(FUNC_COUNT, sizeof(Func_t));
	
	return funcArr; 
}

int FuncArrDtor(Func_t* funcArr)
{
	free(funcArr);

	return OK;
}

int ParseCall(TreeNode_t* node, Program_t* program)
{
	int isDefFunc    = 0;
	size_t funcParam = 0;
	size_t callParam = 0;
	

	for (size_t index = 0; index < program->funcCount; index++)
	{
		if (STR_EQ((program->funcArr[index]).name, node->left->name))
		{
			isDefFunc = 1; 
			funcParam = (program->funcArr[index]).parCount;
			break;	
		}
	}
	
	if (!isDefFunc)
	{
		UNDEF_FUNC(node->left);
	}

	
	if (node->left->left)
		callParam = CountParams(node->left->left);
	
	if (funcParam != callParam)
	{
		return BAD_CALL;
	}
	
	VarTable_t* curTable = GetTableFromStk(program->tabStk, program->tabStk->size - 1);
	fprintf(program->asmFile, "\nPUSH RAX\n"
							  "PUSH %lu\n"
							  "ADD\n"
							  "POP RAX\n", curTable->size);

	for (int index = (int) callParam - 1; index >= 0; index--)
	{
		fprintf(program->asmFile, "POP [%d+RAX]\n", index);
	}

	fprintf(program->asmFile, "\nCALL :%s\n"
							  "\nPUSH RAX\n"
							  "PUSH %lu\n"
							  "SUB\n"
							  "POP RAX\n", node->left->name, curTable->size);

	return OK;
}

size_t CountParams(TreeNode_t* node)
{
	TreeNode_t* curNode = node;
	size_t parCount = 0;

	while (curNode)
	{
		parCount++;
		curNode = curNode->right;
	}

	return parCount;
}

VarTable_t* VarTableCtor(Stack_t* stk)
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

size_t ParseParams(TreeNode_t* node, VarTable_t* table)
{
	TreeNode_t* curNode = node;
	size_t parCount = 0;

	while (curNode)
	{
		parCount++;

		Variable_t param = {};
		param.name      = curNode->left->name;
		param.pos       = table->size;
		table->arr[table->size] = param;

		table->size++;
		curNode = curNode->right;
	}

	return parCount;
}

int ParsePrintf(TreeNode_t* node, Program_t* program)
{
	TreeNode_t* curNode = node;
	while (curNode)
	{
		if (!CheckForVar(node->left, program->tabStk))
		{
			UNDEF_VAR(node->left);
		}
		
		int varPos = GetVarPos(node->left, program->tabStk);
		fprintf(program->asmFile, "PUSH [%d+RAX]\n" 
								  "OUT\n", varPos);

		curNode = curNode->right;
	}
	
	return OK;
}

int ParseScanf(TreeNode_t* node, Program_t* program)
{
	TreeNode_t* curNode = node;
	while (curNode)
	{
		if (!CheckForVar(node->left, program->tabStk))
		{
			UNDEF_VAR(node->left);
		}
		
		int varPos = GetVarPos(node->left, program->tabStk);
		fprintf(program->asmFile, "INP\n" 
								  "POP [%d+RAX]\n", varPos);

		curNode = curNode->right;
	}
	
	return OK;
}	

int ParseDef(TreeNode_t* node, Program_t* program)
{
	if (!CheckForVar(node->right, program->tabStk))
	{
		UNDEF_VAR(node->right);
	}

	if (CheckForVar(node->left, program->tabStk))
	{
		ALREADY_DEF_VAR(node->left);
	}
	
	Variable_t newVar  = {}; 
	VarTable_t* curTable = GetTableFromStk(program->tabStk,  program->tabStk->size - 1);

	newVar.name       = node->left->name;
	newVar.pos        = curTable->size;
		
	curTable->arr[curTable->size] = newVar;
	curTable->size += 1;
		
	CountExpression(node->right, program);
	fprintf(program->asmFile, "//%s\n", node->left->name);
	fprintf(program->asmFile, "POP [%lu+RAX]\n\n", newVar.pos);
	
	return OK;
}

int ParseAssign(TreeNode_t* node, Program_t* program)
{
	if (!CheckForVar(node->right, program->tabStk))
	{
		UNDEF_VAR(node->right);
	}
	
	if (!CheckForVar(node->left, program->tabStk))
	{
		UNDEF_VAR(node->left);
	}
	
	int pos = GetVarPos(node->left, program->tabStk);
	CountExpression(node->right, program);
	
	fprintf(program->asmFile, "//%s\n", node->left->name);
	fprintf(program->asmFile, "POP [%d+RAX]\n", pos);
	
	return OK;
}

int ParseIf(TreeNode_t* node, Program_t* program)
{
	if (!CheckForVar(node->left, program->tabStk))
	{
		UNDEF_VAR(node->right);
	}
	
	CountExpression(node->left, program);
	fprintf(program->asmFile, "PUSH 0\n");
	
	if (node->right->type != Type_ELSE)
	{
		fprintf(program->asmFile, "\nJE :if_%lu\n", program->ifCounter);

		ParseStatement(node->right, program);

		VarTable_t* curTable = GetTableFromStk(program->tabStk, program->tabStk->size - 1);
		VarTableDtor(curTable);
		StackPop(program->tabStk);			
		
		curTable = GetTableFromStk(program->tabStk, program->tabStk->size - 1);
		fprintf(program->asmFile, "PUSH %lu\n", curTable->raxPos);
		fprintf(program->asmFile, "POP RAX\n");	

		fprintf(program->asmFile, "\nif_%lu:\n", program->ifCounter++);
	}
	else
	{
		fprintf(program->asmFile, "\nJE :else_%lu\n", program->ifCounter);
	
		ParseStatement(node->right->left, program);
		VarTable_t* curTable = GetTableFromStk(program->tabStk, program->tabStk->size - 1);
		VarTableDtor(curTable);
		StackPop(program->tabStk);			
		
		curTable = GetTableFromStk(program->tabStk, program->tabStk->size - 1);
		fprintf(program->asmFile, "PUSH %lu\n", curTable->raxPos);
		fprintf(program->asmFile, "POP RAX\n");	
		
		fprintf(program->asmFile, "\nJMP :if_%lu\n", program->ifCounter);
		fprintf(program->asmFile, "\nelse_%lu:\n", program->ifCounter);

		ParseStatement(node->right->right, program);
		
		curTable = GetTableFromStk(program->tabStk, program->tabStk->size - 1);
		VarTableDtor(curTable);
		StackPop(program->tabStk);			
		
		curTable = GetTableFromStk(program->tabStk, program->tabStk->size - 1);
		fprintf(program->asmFile, "PUSH %lu\n", curTable->raxPos);
		fprintf(program->asmFile, "POP RAX\n");	
		
		fprintf(program->asmFile, "\nif_%lu:\n", program->ifCounter++);	
	}
	
	return OK;
}

int ParseWhile(TreeNode_t* node, Program_t* program)
{
	if (!CheckForVar(node->left, program->tabStk))
	{
		UNDEF_VAR(node->right);
	}
	
	fprintf(program->asmFile, "\nwhile_%lu:\n", program->whileCounter);

	CountExpression(node->left, program);
	fprintf(program->asmFile, "PUSH 0\n");

	fprintf(program->asmFile, "\nJE :while_no_%lu\n", program->whileCounter);

	ParseStatement(node->right, program);
	
	VarTable_t* curTable = GetTableFromStk(program->tabStk, program->tabStk->size - 1);
	VarTableDtor(curTable);
	StackPop(program->tabStk);			
	
	curTable = GetTableFromStk(program->tabStk, program->tabStk->size - 1);
	fprintf(program->asmFile, "PUSH %lu\n", curTable->raxPos);
	fprintf(program->asmFile, "POP RAX\n");	
	
	fprintf(program->asmFile, "\nJMP while_%lu:\n", program->whileCounter);	
	fprintf(program->asmFile, "\nwhile_no_%lu:\n",  program->whileCounter++);	
	
	return OK;
}

int ParseRet(TreeNode_t* node, Program_t* program)
{
	if (!CheckForVar(node->left, program->tabStk))
	{
		UNDEF_VAR(node->right);
	}	

	int isInLocal = 0;
	TreeNode_t* curNode = node;
	while (curNode->type != Type_FUNC)
	{
		if (curNode->type == Type_IF    ||
			curNode->type == Type_ELSE  ||
			curNode->type == Type_WHILE)
		{
			isInLocal = 1;
		}

		curNode = curNode->parent;
	}

	CountExpression(node->left,  program);

	VarTable_t* curTable = GetTableFromStk(program->tabStk, program->tabStk->size - 2);
	if (isInLocal)
	{
		fprintf(program->asmFile, "POP RCX\n"
								  "PUSH RAX\n"
								  "PUSH %lu\n"
								  "SUB\n"
								  "POP RAX\n", curTable->size);
	}

	fprintf(program->asmFile, "\nRET\n\n");

	return OK;
}

void CountExpression(TreeNode_t* node, Program_t* program)
{
	if (node->left)  CountExpression(node->left, program);
	if (node->right) CountExpression(node->right, program);
	
	if (node->type == Type_NUM)
	{
		fprintf(program->asmFile, "PUSH %lg\n", node->numVal);
	}
	else if (node->type == Type_VAR)
	{
		int varPos = GetVarPos(node, program->tabStk);
		fprintf(program->asmFile, "PUSH [%d+RAX]\n", varPos);
	}
	else if (node->type == Type_CALL)
	{
		ParseCall(node, program);
		fprintf(program->asmFile, "PUSH RCX\n");
	}
	
	if (node->type == Type_OP)
	{
		switch (node->opVal)
		{
			case Op_Add: fprintf(program->asmFile, "ADD\n"); break;
			case Op_Sub: fprintf(program->asmFile, "SUB\n"); break;
			case Op_Mul: fprintf(program->asmFile, "MUL\n"); break;
			case Op_Div: fprintf(program->asmFile, "DIV\n"); break;
			case Op_Pow: fprintf(program->asmFile, "POW\n"); break;
			case Op_Sin: fprintf(program->asmFile, "ADD\n"); break;
			case Op_Cos: fprintf(program->asmFile, "COS\n"); break;
			case Op_IsBt: fprintf(program->asmFile, "ISBT\n"); break;
			case Op_Ln:  fprintf(program->asmFile, "LN\n"); break;
			
			default: break;
		}
	}
}


int CheckForVar(TreeNode_t* node, Stack_t* stk)
{
	if (node->left)
	{
		if (!CheckForVar(node->left, stk)) return 0;
	}
	
	if (node->right)
	{
		if (!CheckForVar(node->right, stk)) return 0;
	}

	size_t stkIndex      =  stk->size - 1;
	VarTable_t* curTable = GetTableFromStk(stk, stkIndex);
	
	if (node->type == Type_VAR)
	{
		while (stkIndex >= 0)
		{
			for (size_t varIndex = 0; varIndex < curTable->size; varIndex++)
			{
				if (STR_EQ(node->name, (curTable->arr[varIndex]).name))
				{
					return 1; //if we met the variable
				}
			}
			
			if (stkIndex == 0)
				break;
	
			stkIndex--;
			curTable = GetTableFromStk(stk, stkIndex);
		}

		return 0; //if variable is not defined
	}
		
	return 1;
}
				
VarTable_t* GetTableFromStk(Stack_t* stk, size_t index)
{
	return stk->data[index];
}


int GetVarPos(TreeNode_t* node, Stack_t* stk)
{
	size_t stkIndex      = stk->size - 1;
	VarTable_t* curTable = GetTableFromStk(stk, stkIndex);
	size_t       lastRax = curTable->raxPos;

	while (stkIndex >= 0)
	{
		for (size_t varIndex = 0; varIndex < curTable->size; varIndex++)
		{
			if (STR_EQ(node->name, (curTable->arr[varIndex]).name))
			{
				return (int)(curTable->arr[varIndex]).pos + (int)curTable->raxPos - (int)lastRax;
			}
		}
		
		if (stkIndex == 0)
		{
			break;
		}

		stkIndex--;
		curTable = GetTableFromStk(stk, stkIndex);
	}
	return POISON_PTR;
}

