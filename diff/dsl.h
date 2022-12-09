#define DL DiffTree(node->left, var)
#define DR DiffTree(node->right, var)
#define CL TrNodeCopy(node->left)
#define CR TrNodeCopy(node->right)

#define MAKE_NUM(num) MakeNode(Type_NUM, Op_Null, num, NULL, NULL, NULL)
#define MAKE_X		  MakeNode(Type_VAR, Op_Null, 0, "x", NULL, NULL)

#define ADD(L, R) MakeNode(Type_OP, Op_Add, 0, NULL, L, R)
#define SUB(L, R) MakeNode(Type_OP, Op_Sub, 0, NULL, L, R)
#define MUL(L, R) MakeNode(Type_OP, Op_Mul, 0, NULL, L, R)
#define DIV(L, R) MakeNode(Type_OP, Op_Div, 0, NULL, L, R)
#define POW(L, R) MakeNode(Type_OP, Op_Pow, 0, NULL, L, R)
#define LN(R)     MakeNode(Type_OP, Op_Ln,  0, NULL, NULL, R)   		
#define SIN(R)	  MakeNode(Type_OP, Op_Sin, 0, NULL, NULL, R)
#define COS(R)    MakeNode(Type_OP, Op_Cos, 0, NULL, NULL, R)


