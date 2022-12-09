#include "tree.h"

extern FILE* LogFile;

int TreeCtor(Tree_t* tree)
{
	if (tree == NULL) return TREE_NULL_PTR;
	
	tree->root = (TreeNode_t*) calloc(1, sizeof(TreeNode_t));
	if (tree->root == NULL)
		return TREE_NULL_PTR;

	tree->root->type   = Type_NULL;
	tree->root->opVal  = Op_Null;
	tree->root->numVal = 0;
	tree->root->left   = NULL;
	tree->root->right  = NULL;
	tree->root->parent = NULL;

	tree->size   = 1;
	tree->status = TREE_IS_OK;

	return TREE_IS_OK;
}

int TreeDtor(Tree_t* tree)
{
	if (tree == NULL)								  return TREE_NULL_PTR;
	if (TreeIsDestructed(tree) == TREE_IS_DESTRUCTED) return TREE_IS_DESTRUCTED;
	
	TrNodeRemove(tree, tree->root);
	tree->size    = DEAD_SIZE;

	return TREE_IS_DESTRUCTED;
}

int TreeIsDestructed(Tree_t* tree)
{
	if (tree       == NULL)      return TREE_NULL_PTR;
	if (tree->size == DEAD_SIZE) return TREE_IS_DESTRUCTED;

	return TREE_UB;
}

int TreeIsEmpty(Tree_t* tree)
{
	if (tree       == NULL)                    return TREE_NULL_PTR;
	if (tree->root == NULL && tree->size == 0) return TREE_IS_EMPTY;

	return TREE_UB;
}

TreeNode_t* MakeNode(NodeType type, OperationType opVal, double numVal, const char* name, TreeNode_t* left, TreeNode_t* right, size_t lineIndex)
{
	TreeNode_t* newNode = (TreeNode_t*) calloc(1, sizeof(TreeNode_t));
	if (newNode == NULL)
        return NULL;
	
	newNode->type      = type;
	newNode->opVal     = opVal;
	newNode->numVal    = numVal;
	newNode->lineIndex = lineIndex;
	
	if (name)
		strcpy(newNode->name, name);

	newNode->left  = left;
	newNode->right = right;

	return newNode;
}

TreeNode_t* TrNodeInsert(Tree_t* tree, TreeNode_t* parent, InsMode insMode)
{
	if (tree	== NULL)				 	 return NULL;
	if (parent  == NULL)					 return NULL;
	if (insMode != LEFT && insMode != RIGHT) return NULL;
	
	TreeNode_t* newNode = (TreeNode_t*) calloc(1, sizeof(TreeNode_t));
	if (newNode == NULL)
        return NULL;
	
	if (insMode == LEFT)
		parent->left  = newNode;
	else
		parent->right = newNode;


    tree->size++;
	
	TreeNodeCtor(newNode);
    newNode->parent = parent;

    return newNode;
}

int TreeNodeCtor(TreeNode_t* newNode)
{
	if (newNode == NULL) return NODE_NULL_PTR;

	newNode->type   = Type_NULL;
    newNode->numVal = 0;
    newNode->opVal  = Op_Null;

    newNode->left  = NULL;
    newNode->right = NULL;
	
	return TREE_IS_OK;
}

int TrNodeRemove(Tree_t* tree, TreeNode_t* node)
{
	if (tree == NULL) return TREE_NULL_PTR;
	if (node == NULL) return NODE_NULL_PTR;
	
	if (node->left)
	{
		TrNodeRemove(tree, node->left);
	}
	if (node->right)
	{
		TrNodeRemove(tree, node->right);
	}
	
	free(node); 
	node = nullptr;

	--tree->size;

	return TREE_IS_OK;
}

int DeleteNode(TreeNode_t* node)
{
	if (node == NULL) return NODE_NULL_PTR;
	
	if (node->left)
	{
		DeleteNode(node->left);
	}
	if (node->right)
	{
		DeleteNode(node->right);
	}
	
	free(node); 
	node = nullptr;

	return TREE_IS_OK;
}

int TreeVerify(Tree_t* tree)
{
    if (tree			       == NULL)               return TREE_NULL_PTR;
    if (TreeIsEmpty(tree)      == TREE_IS_EMPTY)      return TREE_IS_EMPTY;
    if (TreeIsDestructed(tree) == TREE_IS_DESTRUCTED) return TREE_IS_DESTRUCTED;
	if (tree->root             == NULL)               return TREE_ROOT_IS_NULL;

    int status = TREE_IS_OK;
 
    return status;
}

int NodeDump(TreeNode_t* node, int* nodeCount, FILE* file)
{
	ASSERT(node != NULL && nodeCount != NULL && file != NULL);
	
	int leftNum  = 0;
	int rightNum = 0;
	
	if (node->left != NULL)
	{
		leftNum = NodeDump(node->left, nodeCount, file);
	}

	if (node->right != NULL)
	{
		rightNum = NodeDump(node->right, nodeCount, file);
	}

	fprintf(file, "\"node%d\" [fillcolor=", *nodeCount);
	
	switch (node->type)
	{
		case Type_OP:   fprintf(file, "\"#AADEE2\", label=\""); break;
		case Type_VAR:  fprintf(file, "\"#FFB2D0\", label=\"");   break;
		case Type_NUM:  fprintf(file, "\"#98FF98\", label=\"");   break;
		case Type_STAT: fprintf(file, "\"#C0C0C0\", label=\""); break;

		default: fprintf(file, "\"#EFAF8C\", label=\"");
	}
	
	if (node->type == Type_OP)
	{
		switch (node->opVal)
		{
			case Op_Add: fprintf(file, "+");   break;
			case Op_Sub: fprintf(file, "-");   break;
			case Op_Mul: fprintf(file, "*");   break;
			case Op_Div: fprintf(file, "/");   break;
			case Op_Log: fprintf(file, "log"); break;
			case Op_Ln:  fprintf(file, "ln");  break;
			case Op_Sin: fprintf(file, "sin"); break;
			case Op_Cos: fprintf(file, "cos"); break;
			case Op_Pow: fprintf(file, "^");   break;

			default: fprintf(file, "?");
		}
	}
	else if (node->type == Type_NUM)
	{
		fprintf(file, "%lg", node->numVal);
	}
	else
	{
		fprintf(file, "%s", node->name);
	}
	
	fprintf(file, "\"]\n");

	if (node->left != NULL)
	{
		fprintf(file, "\"node%d\" -- \"node%d\"\n", *nodeCount, leftNum);
	}
	
	if (node->right != NULL)
	{
		fprintf(file, "\"node%d\" -- \"node%d\"\n", *nodeCount, rightNum);
	}
	
	++(*nodeCount);

	return *nodeCount - 1;
}

void TreeDump(Tree_t* tree)
{   
    ASSERT(tree != NULL);

    fprintf(LogFile, "\n<hr>\n");
   
    fprintf(LogFile, "<h1> Tree status: </h1>\n");

    FILE* DumpFile = fopen("obj/dump", "w+");
	
	fprintf(DumpFile, "strict graph {\n");
    fprintf(DumpFile, "\trankdir = TB\n");
    fprintf(DumpFile, "\t\"info\" [shape = \"record\", style = \"filled\", fillcolor = \"grey\", label = \"size = %lu\"];\n", tree->size);
	fprintf(DumpFile, "node [color=black, shape=box, style=\"rounded, filled\"];\n");

	int nodeCount = 1;

	NodeDump(tree->root, &nodeCount, DumpFile);

    fprintf(DumpFile, "}\n");
    
    fclose(DumpFile);

    static char pngIndex        = 1;
    char dumpName[STR_MAX_SIZE] = "";

    MakePngName(dumpName, pngIndex);

    char buff[100] = "";
    sprintf(buff, "dot obj/dump -T png -o %s", dumpName);
    system(buff);
	
    fprintf(LogFile, "<img src = %s>\n", dumpName + 4);

	char showpic[100] = "";
	sprintf(showpic, "eog %s\n", dumpName);
	//system(showpic);

    ++pngIndex;

}

void MakePngName(char* name, char num)
{
    ASSERT(name != NULL);
	
	sprintf(name, "obj/dump%03d.png", num);
}

int TreeUpdate(Tree_t* tree, TreeNode_t* node)
{
	if (node == NULL || tree == NULL) return TREE_NULL_PTR;

	if (node && node == tree->root) 
		tree->size = 0;

	++tree->size;
	
	if (node != nullptr && node->left != nullptr)
	{
		node->left->parent = node;
		TreeUpdate(tree, node->left);
	}

	if (node != nullptr && node->right != nullptr)
	{
		node->right->parent = node;
		TreeUpdate(tree, node->right);
	}
	
	if (node->left && node->right && node->right->type == Type_NUM && node->left->type != Type_NUM &&
		(node->opVal == 3 || node->opVal == 0))
	{
		TreeNode_t* tmpNode = node->right;
		node->right = node->left;
		node->left  = tmpNode;
	}
	

	if (node->left == NULL && node->right == NULL) return 0;

	return 0;
}

