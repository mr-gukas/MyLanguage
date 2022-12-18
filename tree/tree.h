#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <assert.h> 
#include "../log/LOG.h"

#define LOG_MODE

#ifdef LOG_MODE
    #define ASSERT(condition)                                       \
        if (!(condition)){                                           \
            fprintf(stderr, "Error in %s:\n"                          \
                            "FILE: %s\n"                               \
                            "LINE: %d\n"                                \
                            "FUNCTION: %s\n",                            \
                   #condition, __FILE__, __LINE__, __PRETTY_FUNCTION__);  \
            abort();}

    #define ASSERT_OK(tree)                                                       \
        if (TreeVerify(tree) != TREE_IS_OK && TreeIsEmpty(tree) != TREE_IS_EMPTY) \
        {                                                                         \
            TreeDump(tree)                                                        \
            ASSERT(0 && "Crashed tree")                                           \
        }

#else
    #define ASSERT(condition) ;
    #define ASSERT_OK(tree)   ;
#endif

typedef char Val_t;

enum InsMode
{
	LEFT  = -1,
	RIGHT =  1,
};


const size_t DEAD_SIZE      = 0xDEADBEAF;
const size_t STR_MAX_SIZE   = 50;
const size_t VAR_SIZE       = 10;

enum TreeStatus
{
    TREE_IS_OK              = 0 << 0,
    TREE_NULL_PTR           = 1 << 0,
    TREE_DATA_NULL_PTR      = 1 << 1,
    CANNOT_ALLOCATE_MEMORY  = 1 << 2,
    TREE_IS_DESTRUCTED      = 1 << 3,
    TREE_IS_EMPTY           = 1 << 4,
    TREE_ROOT_IS_NULL       = 1 << 5,
    BAD_PRINT_FILE          = 1 << 6,
    TREE_UB                 = 1 << 7,
    NODE_NULL_PTR           = 1 << 8,
    BAD_INS_MODE            = 1 << 9,
    BAD_REMOVE              = 1 << 10,
    TREE_RUINED             = 1 << 18,
	CANT_MAKE_NODE          = 1 << 19,
};


enum NodeType
{
	Type_NULL = 0,
	Type_OP,
	Type_VAR,
	Type_NUM,
	Type_FUNC,
	Type_PARENTHESIS,
	Type_ELSE,
	Type_IF,
	Type_THEN,
	Type_WHILE,
	Type_RETURN,
	Type_STR_END,
	Type_ASSIGN,
	Type_PARAM,
	Type_INIT,
	Type_CALL,
	Type_COMMA,
	Type_STAT,
	Type_DEF,
	Type_PRINTF,
	Type_SCANF,
	Type_RET_VAL,
	Type_FUNC_NAME,
};

enum OperationType
{
	Op_Null = -1,

	Op_Add  =  0,
	Op_Sub  =  1,

	Op_Mul  =  3,
	Op_Div  =  4,
	
	Op_Log  =  6,
	Op_Ln   =  7,

	Op_Sin  =  9,
	Op_Cos  = 10,

	Op_Pow  = 12,
	Op_IsBt = 14,
};

struct TreeNode_t
{
	NodeType type;
	
	OperationType opVal;
	double        numVal;
	char          name[VAR_SIZE];
	size_t        lineIndex;
	
	TreeNode_t*  parent;
	TreeNode_t*	 left;
	TreeNode_t*	 right;
};

struct Tree_t
{
	TreeNode_t* root;
	size_t      size;
	int  status;
};

void MakePngName(char* name, char num);
void TreeDump(Tree_t* tree);
int TreeVerify(Tree_t* tree);
int TreePrint(TreeNode_t* root, FILE* stream, size_t tabCount=0);
int TrNodeRemove(Tree_t* tree, TreeNode_t* node);
int TreeIsEmpty(Tree_t* tree);
int TreeIsDestructed(Tree_t* tree);
int TreeDtor(Tree_t* tree);
int TreeCtor(Tree_t* tree);
int DeleteNode(TreeNode_t* node);

TreeNode_t* MakeNode(NodeType type, OperationType opVal, double numVal, const char* name, TreeNode_t* left, TreeNode_t* right, size_t lineIndex = 0);

TreeNode_t* TrNodeInsert(Tree_t* tree, TreeNode_t* parent, InsMode insMode);

int TreeNodeCtor(TreeNode_t* newNode);

int NodeDump(TreeNode_t* node, int* nodeCount, FILE* file);

int TreeUpdate(Tree_t* tree, TreeNode_t* node);
