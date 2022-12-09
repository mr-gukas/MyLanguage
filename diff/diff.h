#pragma once
#include "tree/tree.h"
#include "stack/stack.h"
#include "filework/filework.h"
#include "log/LOG.h"
#include <stdarg.h>
#include "dsl.h"
#include <math.h>
#include <assert.h>
#include "book.h"

const size_t TEXT_SIZE = 100;

enum AkinatorStatus
{
	STATUS_OK         = 0 << 0,
	NULL_EXPRESSION   = 1 << 0,
	NULL_TREE         = 1 << 1,
	BAD_NODE          = 1 << 2,
	WRONG_DATA        = 1 << 3, 
	NULL_NODE         = 1 << 4,
	NON_EXISTABLE_OBJ = 1 << 5,
};

struct VarValue_t
{
	char   name[VAR_SIZE];
	double value;
	double varErr;
};

struct Expression_t
{
	Tree_t*     tree;
	size_t		derOrd;
	double		point;
	size_t		macOrd;
	size_t      varCount;
	VarValue_t* varArr;
};

int DataDownload(Expression_t* exp);

int MakeNodeValue(char* value, const char* data);

int DataUpload(Tree_t* tree);

int ReadTree(Tree_t* tree, const char* expression, size_t size);

static int ReadNodeValue(TreeNode_t* curNode, const char* expression);

TreeNode_t* TrNodeCopy(TreeNode_t* node);

int EndTexPrint(FILE* texFile);

int StartTexPrint(FILE* texFile);

int TrNodesPrint(const TreeNode_t* node, FILE* texFile);

int TreeTexPrint(const Tree_t* tree, FILE* texFile);

int IsInTree(TreeNode_t* node, const char* value);
TreeNode_t* DiffTree(TreeNode_t* node, const char* var);

int TreeSimplify(Tree_t* tree);

int ReadVarValue(Expression_t* exp, TEXT* data);
int TreeSimplifyConst(Tree_t* tree, TreeNode_t* node);
int SimplifyConst(TreeNode_t* node, Tree_t* tree);
int TreeSimplifyNeutral(Tree_t* tree, TreeNode_t* node);
int SimplifyNeutral(TreeNode_t* node, Tree_t* tree);

double CalcValue(Tree_t* tree, double point, const char* var, FILE* texFile);

Tree_t* DiffExpression(Tree_t* tree, const char* var, size_t derOrd, FILE* texFile);

int PutValueInPoint(TreeNode_t* tree, const char* var, double point);

int Maclaurin(Expression_t* exp, FILE* texFile);
size_t factorial(size_t num);

int TangentEquation(Expression_t* exp, FILE* texFile);
int BuildGraph(Expression_t* exp, FILE* texFile);
void MakeBook(Expression_t* exp, FILE* texFile);

int CalculateError(Expression_t* exp, FILE* texFile);

TreeNode_t* GetNumber(char** str);
TreeNode_t* GetVar(char** str);
OperationType isTextFunc(char** str);
TreeNode_t* GetTextFunc(char** str);
TreeNode_t* GetBracket(char** str);
TreeNode_t* GetPower(char** str);
TreeNode_t* GetT(char** str);
TreeNode_t* GetE(char** str);
TreeNode_t* GetG(char** str);
