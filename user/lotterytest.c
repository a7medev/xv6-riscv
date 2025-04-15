#include "kernel/types.h"
#include "user/user.h"

#define N 10

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
            for (;;) {
                // Do nothing, just busy looping to consume CPU.
            }
            break;
        }

        settickets(tickets);
        tickets *= 2;
    }
}

int
main(void)
{
    lotterytest();
    exit(0);
}
