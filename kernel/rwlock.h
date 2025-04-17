struct rwlock {
    struct sleeplock rwlk; // the actual sleep lock acquired by writer and first reader
    struct spinlock lk;    // protects everything below
    int readers;           // number of readers
    int writer;            // process ID of writer
};
