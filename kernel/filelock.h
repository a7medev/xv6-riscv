struct flock {
    struct spinlock lk;       // protects everything below
    int locked;               // whether the lock is locked
    int readers;              // number of readers
    int writer;               // process ID of writer
    BITARRAY(holders, NPROC); // bitarray with BIT(holders, i): process[i] holds filelock
};
