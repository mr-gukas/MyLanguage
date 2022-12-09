#define CANARY_GUARD 00
#define HASH_GUARD   00

typedef double Elem_t;
typedef unsigned long long Canary_t;


#if CANARY_GUARD
const Canary_t LEFT_CANARY       = 0xBAADF00DBAADF00D;
const Canary_t RIGHT_CANARY      = 0xF00DBAADF00DBAAD;
const Canary_t DATA_LEFT_CANARY  = 0xDEADBEEFDEADBEEF;
const Canary_t DATA_RIGHT_CANARY = 0xBEEFDEADBEEFDEAD;
#endif

#if HASH_GUARD
const size_t   HASH_DESTRUCT     = 0xDEADBABE;
#endif

const size_t   SIZE_DESTRUCT     = 0xDEAD;
const size_t   CAPACITY_DESTRUCT = 0xC0FE;
const Elem_t   POISON            = 0;
const int      DATA_DESTRUCT     = 0xDED0;
const Canary_t CANARY_DESTRUCT   = 0xDEADBABADEADBABA;
const size_t   BASED_CAPACITY    = 10;

