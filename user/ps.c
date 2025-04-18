#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/pstat.h"
#include "user/user.h"

void
main(void)
{
    struct pstat st;
    getpinfo(&st);

    printf("PID     Tickets     Ticks\n");
    for (int i = 0; i < NPROC; i++) {
        if (st.inuse[i]) {
            printf("%d     %d     %d\n", st.pid[i], st.tickets[i], st.ticks[i]);
        }
    }

    exit(0);
}
