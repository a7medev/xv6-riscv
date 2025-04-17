struct rwlock {
    struct spinlock lk;    // protects everything below
    int locked;            // whether the lock is locked
    int readers;           // number of readers
    int writer;            // process ID of writer
};
