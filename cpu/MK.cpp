#include <stdlib.h>

int main(void)
{
    system("run/asm ../obj/arm.asm ../obj/bin.txt");
    system("run/cpu ../obj/bin.txt");
    
    return 0;
}

