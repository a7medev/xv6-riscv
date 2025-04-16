#include "kernel/types.h"
#include "user/user.h"

#define N 5

void
lotterytest(void)
{
    int tickets = 2;
    int i;
    for (i = 0; i < N; ++i) {
        int pid = fork();
        if (pid < 0) {
            printf("fork failed\n");
            exit(1);
        }

        if (pid > 0) {
            break;
        }

        settickets(tickets);
        tickets *= 2;
    }

    for (;;) {
        // Do nothing, just busy looping to consume CPU.
    }
}

int
main(void)
{
    lotterytest();
    exit(0);
}
