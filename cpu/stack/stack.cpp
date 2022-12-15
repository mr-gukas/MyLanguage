#include "stack.h"
 
extern FILE* PrintFile;

void print(FILE* file, int param)
{
    fprintf(file, "%d", param);
}

void print(FILE* file, char param)
{
    fprintf(file, "%c", param);
}

void print(FILE* file, char* param)
{
    fprintf(file, "%s", param);
}

void print(FILE* file, double param)
{
    fprintf(file, "%lf", param);
}

Elem_t* StackResize(Stack_t* stk, CapacityMode capMode)
{   
    if (stk == NULL) return NULL;

    //ASSERT_OK(stk);

    size_t updCapacity = 0;
   
    if (capMode == UP)
    {
        if (stk->capacity == 0 || stk->size >= stk->capacity)
        {
            updCapacity  = (stk->capacity == 0) ? BASED_CAPACITY : stk->capacity * 2;
        }
        else
            return stk->data;
    }
    else if (capMode == DOWN)
    {
        if (stk->capacity > BASED_CAPACITY && stk->size <= stk->capacity * 3 / 8)
        {
            updCapacity = stk->capacity / 2;
        }
        else
            return stk->data;
    }
    else
    {
        stk->status  |= STACK_BAD_RESIZE;
        StackDump(stk); 

        return NULL;
    }

    char* dataptr = NULL;

    if (stk->capacity == 0 && capMode == UP)
    {
#if CANARY_GUARD
        dataptr = (char*) calloc(1, updCapacity * sizeof(Elem_t) +  2 * sizeof(Canary_t));
#else
        dataptr = (char*) calloc(1, updCapacity * sizeof(Elem_t));
#endif
    }
    else if (stk->capacity == 0 && capMode == DOWN)
    {
        stk->status  |= STACK_BAD_RESIZE;
        StackDump(stk);

        return NULL;
    }
    else
    {
#if CANARY_GUARD
        dataptr = (char*) realloc((char*) stk->data - sizeof(Canary_t), updCapacity * sizeof(Elem_t) +  2 * sizeof(Canary_t));
#else
        dataptr = (char*) realloc((char*) stk->data,                    updCapacity * sizeof(Elem_t));
#endif
    }

    if (dataptr == NULL)
    {
        stk->status |= CAN_NOT_ALLOCATE_MEMORY;
        StackDump(stk);
        
        return NULL;
    }
#if CANARY_GUARD
    stk->data = (Elem_t*) (dataptr + sizeof(Canary_t));
    *((Canary_t*)  dataptr                                                   ) = DATA_LEFT_CANARY;
    *((Canary_t*) (dataptr + sizeof(Canary_t) + sizeof(Elem_t) * updCapacity)) = DATA_RIGHT_CANARY; 
#else
    stk->data = (Elem_t*) dataptr;
#endif
    
    stk->capacity = updCapacity;
    
    for (size_t index = stk->size; index < stk->capacity; ++index)
    {
#if CANARY_GUARD 
        stk->data[index] = POISON;
#else
        stk->data[index] = 0;
#endif
    }

#if HASH_GUARD
    stk->stackHash = SetStackHash(stk);
    stk->dataHash  = SetStackDataHash(stk);
#endif

    //ASSERT_OK(stk);

    return stk->data;
}

Stack_t* StackCtorFunc(Stack_t*   stk,                size_t     capacity,           const char name[MAX_STR_SIZE], size_t line,
                       const char file[MAX_STR_SIZE], const char func[MAX_STR_SIZE])
{  
    if (stk == NULL) return NULL;
    
    (stk->info).stackName = name;         
    (stk->info).stackFunc = func;
    (stk->info).stackFile = file;            
    (stk->info).stackLine = line;

#if CANARY_GUARD
    stk->leftCanary  = LEFT_CANARY;
    stk->rightCanary = RIGHT_CANARY;
#endif
    
    if (capacity == 0)
    {
        stk->size        = 0;
        stk->capacity    = capacity;
        stk->status      = STACK_IS_EMPTY;
    
#if HASH_GUARD
        stk->stackHash   = SetStackHash(stk);
        stk->dataHash    = 0;
#endif        
        return stk;
    }
    
    capacity = (capacity % BASED_CAPACITY != 0) ? ((capacity / BASED_CAPACITY) + 1) * BASED_CAPACITY : capacity;
    
#if CANARY_GUARD
    char* dataptr = (char*) calloc(1, capacity * sizeof(Elem_t) + 2 * sizeof(Canary_t));
#else
    char* dataptr = (char*) calloc(1, capacity * sizeof(Elem_t));
#endif

    if (dataptr == NULL)
    {
        stk->status |= CAN_NOT_ALLOCATE_MEMORY;  
        
        return stk;
    }

#if CANARY_GUARD
    stk->data = (Elem_t*) (dataptr + sizeof(Canary_t));
    *((Canary_t*)  dataptr)                                                 = DATA_LEFT_CANARY;
    *((Canary_t*) (dataptr + sizeof(Canary_t) + sizeof(Elem_t) * capacity)) = DATA_RIGHT_CANARY; 
    
    for (size_t index = 0; index < capacity; ++index)
    {
        stk->data[index] = (Elem_t) POISON;
    }
    
#else
    stk->data = (Elem_t*) dataptr;
#endif

    stk->size     = 0;
    stk->capacity = capacity;
    stk->status   = STACK_STATUS_OK;
   
#if HASH_GUARD
    stk->stackHash = SetStackHash(stk);
    stk->dataHash  = SetStackDataHash(stk);
#endif 

   // ASSERT_OK(stk);

    return stk;
}

StackStatus StackDtor(Stack_t* stk)
{  
    if (stk == NULL)
    {   
        StackDump(stk);

        return STACK_NULL_PTR;
    }
    
    if (StackIsDestructed(stk) == STACK_IS_DESTRUCTED)
    {
        StackDump(stk);

        return STACK_IS_DESTRUCTED;
    }

#if CANARY_GUARD
    free(((char*) stk->data - sizeof(Canary_t))); 
#else
    free(stk->data);
#endif

    stk->data        = (Elem_t *) DATA_DESTRUCT;
    stk->size        = SIZE_DESTRUCT;
    stk->capacity    = CAPACITY_DESTRUCT;
    stk->status      = STACK_IS_DESTRUCTED;

#if CANARY_GUARD
    stk->leftCanary  = CANARY_DESTRUCT;
    stk->rightCanary = CANARY_DESTRUCT;
#endif

#if HASH_GUARD
    stk->stackHash   = HASH_DESTRUCT;
    stk->dataHash    = HASH_DESTRUCT;
#endif

    return  (StackStatus) stk->status;
}

StackStatus StackPush(Stack_t* stk, Elem_t value)
{
    if (stk == NULL)
    {
        StackDump(stk);

        return STACK_NULL_PTR;
    }
    
    //ASSERT_OK(stk);
    
    StackResize(stk, UP); 
    if (stk->data == NULL)
    {
        StackDump(stk);
            
        return (StackStatus) stk->status;
    }

    stk->data[stk->size++] = value;

#if HASH_GUARD
    stk->stackHash = SetStackHash(stk);
    stk->dataHash  = SetStackDataHash(stk);
#endif

    //ASSERT_OK(stk);

    return STACK_STATUS_OK;
}

Elem_t StackPop(Stack_t* stk)
{
    if (stk == NULL)
    {
        StackDump(stk);

        return POISON;
    }
    
    //ASSERT_OK(stk);

    if (stk->size == 0)
    {

#if HASH_GUARD
        stk->stackHash = SetStackHash(stk);
#endif
        return POISON;
    }
    
    Elem_t delElem     = stk->data[--stk->size];

#if CANARY_GUARD
    stk->data[stk->size] = POISON;
    stk->stackHash = SetStackHash(stk);
    stk->dataHash  = SetStackDataHash(stk);
#else
    stk->data[stk->size] = 0;
#endif

#if HASH_GUARD
    stk->stackHash = SetStackHash(stk);
    stk->dataHash  = SetStackDataHash(stk);
#endif

    StackResize(stk, DOWN);
    if (stk->data == NULL)
    {
        StackDump(stk);
            
        //return stk->status;
		return NULL;
    }

#if HASH_GUARD
    stk->stackHash = SetStackHash(stk);
    stk->dataHash  = SetStackDataHash(stk);
#endif

    //ASSERT_OK(stk);

    return delElem;
}

void StackDumpFunc(Stack_t* stk, size_t line, const char file[MAX_STR_SIZE], const char func[MAX_STR_SIZE])
{   
    
    fprintf(PrintFile, "\n---------------------------StackDump---------------------------------------\n");
   
    if (stk->status & STACK_NULL_PTR)
    {
        fprintf(PrintFile, "Stack's pointer is null\n");
    }

    fprintf(PrintFile, "Called at %s at %s(%lu)\n", file, func, line);
    fprintf(PrintFile, "Stack[%p] <%s> at %s at %s(%lu)\n", stk, (stk->info).stackName, (stk->info).stackFunc,
                                                  (stk->info).stackFile, (stk->info).stackLine);
    fprintf(PrintFile, "Stack status:\n");
    

    StackVerify(stk);
    
    if (stk->status & STACK_DATA_IS_RUINED)
    {
        fprintf(PrintFile, "\t\t!!!STACK'S DATA IS RUINED!!!\n");
    }
 
    #define StatPrint_(STATUS, text) \
        if (stk->status & STATUS)          \
        {                                   \
            fprintf(PrintFile, #text "\n");    \
        }
    
    StatPrint_(STACK_IS_EMPTY,                 >>>Stack is empty);
    StatPrint_(STACK_IS_DESTRUCTED,            >>>Stack is destructed);
    StatPrint_(STACK_UB,                       >>>Stack has undefined behavior);
    StatPrint_(STACK_BAD_RESIZE,               >>>Stack has resize problem);
    StatPrint_(CAN_NOT_ALLOCATE_MEMORY,        >>>Allocate memory problems);
    StatPrint_(STACK_SIZE_MORE_THAN_CAPACITY,  >>>Stack size more than capacity);
    StatPrint_(STACK_DATA_NULL_PTR,            >>>Stack data pointer is null);

#if CANARY_GUARD
    StatPrint_(STACK_LEFT_CANARY_RUINED,       >>>Stack left canary is ruined);
    StatPrint_(STACK_RIGHT_CANARY_RUINED,      >>>Stack right canary is ruined);
    StatPrint_(STACK_DATA_LEFT_CANARY_RUINED,  >>>Stack data left canary is ruined);
    StatPrint_(STACK_DATA_RIGHT_CANARY_RUINED, >>>Stack data right canary is ruined);
#endif

#if HASH_GUARD
    StatPrint_(STACK_HASH_IS_RUINED,           >>>Stack hash is ruined);
    StatPrint_(STACK_DATA_HASH_IS_RUINED,      >>>Stack data hash is ruined);
#endif

    if (!stk->status)
    {
        fprintf(PrintFile, ">>>Stack is OK\n");
    }
    
    fprintf(PrintFile, "{\n");

#if HASH_GUARD
    fprintf(PrintFile, "    STACK HASH:  %lx    STACK DATA HASH: %lx\n",  stk->stackHash, stk->dataHash);
#endif

#if CANARY_GUARD
    fprintf(PrintFile, "    LEFT CANARY: %llx   RIGHT CANARY:    %llx\n", stk->leftCanary, stk->rightCanary);
#endif

    fprintf(PrintFile, "    data pointer = %p\n", stk->data);
    
    if (StackIsDestructed(stk) == STACK_IS_DESTRUCTED)
    {        
        fprintf(PrintFile, "    size         = %lx\n", stk->size);
        fprintf(PrintFile, "    capacity     = %lx\n", stk->capacity);

    }
    else if (!(stk->status & (STACK_DATA_NULL_PTR | STACK_IS_EMPTY))) 
    {
        fprintf(PrintFile, "    size         = %lu\n", stk->size);
        fprintf(PrintFile, "    capacity     = %lu\n", stk->capacity);

#if CANARY_GUARD
        fprintf(PrintFile, "    DATA LEFT CANARY: %llx   DATA RIGHT CANARY: %llx\n",
                              *((Canary_t*) ((char*) stk->data - sizeof(Canary_t))), 
                              *((Canary_t*) ((char*) stk->data + sizeof(Elem_t) * stk->capacity)));
#endif
        fprintf(PrintFile, "    {\n");
        
        for (size_t index = 0; index < stk->capacity; ++index)
        {
            fprintf(PrintFile, "\t");
            fprintf(PrintFile, (index < stk->size) ? "*" : " ");
            fprintf(PrintFile, "[%lu] = ", index);

            print(PrintFile, stk->data[index]);

#if CANARY_GUARD            
            if (index >= stk->size)
            {
               fprintf(PrintFile, " (POISON)");
            }
#endif
            fprintf(PrintFile, "\n");
        }
        fprintf(PrintFile, "    }\n");
    }

    fprintf(PrintFile, "}\n");
    fprintf(PrintFile, "\n---------------------------------------------------------------------------\n");

    #undef StatPrint_
}

StackStatus StackIsEmpty(Stack_t* stk)
{
   if (stk == NULL)
   {
       return STACK_NULL_PTR;
   }

   if (stk->size     == 0 &&
       stk->capacity == 0)
   {   
       return STACK_IS_EMPTY;
   }

   return STACK_UB;
}

StackStatus StackIsDestructed(Stack_t* stk)
{
   if (stk == NULL)
       return STACK_NULL_PTR;

   if (stk->data      == (Elem_t*) DATA_DESTRUCT && 
       stk->capacity  == CAPACITY_DESTRUCT       && 
       stk->size      == SIZE_DESTRUCT             )
    {
        return STACK_IS_DESTRUCTED;
    }
    
    return STACK_UB;
}
       

int StackVerify(Stack_t* stk)
{
    if (stk == NULL)
        return STACK_NULL_PTR;
    
    if (StackIsEmpty(stk) == STACK_IS_EMPTY)
        return STACK_IS_EMPTY;
    
    if (StackIsDestructed(stk) == STACK_IS_DESTRUCTED)
        return STACK_IS_DESTRUCTED;
    
    int status = STACK_STATUS_OK;
    
    if (stk->size > stk->capacity)
        status |= STACK_SIZE_MORE_THAN_CAPACITY | STACK_DATA_IS_RUINED;

    if (stk->data == NULL && stk->capacity > 0)
        status |= STACK_DATA_NULL_PTR;

#if CANARY_GUARD
    if (stk->leftCanary != LEFT_CANARY)
        status |= STACK_LEFT_CANARY_RUINED | STACK_DATA_IS_RUINED;
    
    if (stk->rightCanary != RIGHT_CANARY)
        status |= STACK_RIGHT_CANARY_RUINED | STACK_DATA_IS_RUINED;
    
    if (*((Canary_t*) ((char*) stk->data - sizeof(Canary_t))) != DATA_LEFT_CANARY)
        status |= STACK_DATA_LEFT_CANARY_RUINED | STACK_DATA_IS_RUINED;
    
    if (*((Canary_t*) ((char*) stk->data + sizeof(Elem_t) * stk->capacity)) != DATA_RIGHT_CANARY)
        status |= STACK_DATA_RIGHT_CANARY_RUINED | STACK_DATA_IS_RUINED;
#endif

#if HASH_GUARD
    if (stk->stackHash != SetStackHash(stk))
        status |= STACK_DATA_IS_RUINED | STACK_HASH_IS_RUINED;

    if ((status & STACK_DATA_IS_RUINED) == 0 && stk->dataHash != SetStackDataHash(stk))
        status |= STACK_DATA_IS_RUINED | STACK_DATA_HASH_IS_RUINED;

    stk->stackHash = SetStackHash(stk);
#endif

    stk->status = status;
    
    return status;
}

#if HASH_GUARD
size_t HashCalculate(char* key, size_t len)
{
    size_t hash = 0, index = 0;

    for (hash = 0, index = 0; index < len; ++index)
    {
        hash += (size_t) key[index];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }

    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;
}

size_t SetStackHash(Stack_t* stk)
{
    if (stk == NULL)
    {
        return (size_t) STACK_NULL_PTR;
    }

    size_t oldStackHash = stk->stackHash;
    size_t oldDataHash  = stk->dataHash;
    stk->stackHash      = 0;   
    stk->dataHash       = 0;

    size_t hash = HashCalculate((char*) stk, sizeof(*stk));

    stk->stackHash = oldStackHash;
    stk->dataHash  = oldDataHash;

    return hash;
}

size_t SetStackDataHash(Stack_t* stk)
{
    if (stk == NULL)
    {
        return (size_t) STACK_NULL_PTR;
    }

    size_t hash = 0;

    if (stk->data != NULL && StackIsDestructed(stk) != STACK_IS_DESTRUCTED && stk->stackHash == SetStackHash(stk))
    {
        hash += HashCalculate((char *) stk->data, stk->capacity * sizeof(Elem_t));
    }
    else
    {
        stk->status |= STACK_DATA_IS_RUINED;
        StackDump(stk);
    }
        
    return hash;
}
#endif
