#include "kernel/types.h"
#include "user/user.h"

void main(void)
{
    int *p = 0;
    printf("%d\n", *p);
    exit(0);
}
