#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#define NITER    10
#define SMALLSZ  4096
#define MEDIUMSZ 16384
#define LARGESZ  65536

void initmem(char *mem, int sz) {
  for (int i = 0; i < sz; i++) {
    mem[i] = (char)(i % 256);
  }
}

// allocates memory of given size and forks
void testread(char *name, int sz, int cow) {
  uint64 start, end, total = 0;
  
  printf("%s with %dB read\n", name, sz);
  
  for (int i = 0; i < NITER; i++) {
    char *mem = sbrk(sz);
    if (mem <= 0) {
      printf("sbrk(%d) failed\n", sz);
      exit(1);
    }
    
    initmem(mem, sz);
    
    start = clock();
    
    int pid;
    if (cow) {
      pid = vfork();
    } else {
      pid = fork();
    }
    
    if (pid < 0) {
      printf("fork/vfork failed\n");
      exit(1);
    }
    
    if (pid == 0) {
      // child process
      // access memory to trigger page faults/COW
      volatile int chksm = 0;
      for (int j = 0; j < sz; j += 1024) { // sample every 1KB
        chksm += mem[j];
      }
      
      // don't print, just use checksum to prevent optimization
      if (chksm == 12345) {
        printf("Unlikely checksum value: %d\n", chksm);
      }
      
      exit(0);
    }

    // parent process
    wait(0);
    end = clock();
    total += end - start;
    
    sbrk(-sz);
  }
  
  printf("  average time: %ld cycles\n", total / NITER);
}

// run a write-heavy workload to trigger COW
void testwrite(char *name, int sz, int cow) {
  uint64 start, end, total = 0;
  
  printf("%s with %dB write\n", name, sz);
  
  for (int i = 0; i < NITER; i++) {
    char *mem = sbrk(sz);
    if (mem <= 0) {
      printf("sbrk(%d) failed\n", sz);
      exit(1);
    }
    
    initmem(mem, sz);
    
    start = clock();
    
    int pid;
    if (cow) {
      pid = vfork();
    } else {
      pid = fork();
    }
    
    if (pid < 0) {
      printf("fork/vfork failed\n");
      exit(1);
    }
    
    if (pid == 0) {
      // modify memory to trigger COW
      for (int j = 0; j < sz; j += 4096) { // modify each page
        mem[j] += 1; // trigger COW
      }
      exit(0);
    }
    
    // Parent process
    wait(0);
    end = clock();
    total += (end - start);

    sbrk(-sz);
  }
  
  printf("  average time: %ld cycles\n", total / NITER);
}

int main() {
  printf("fork vs vfork performance test\n");
  
  // small allocation - read only
  printf("READ ONLY TESTS\n");
  testread("fork small", SMALLSZ, 0);
  testread("vfork small", SMALLSZ, 1);
  
  testread("fork medium", MEDIUMSZ, 0);
  testread("vfork medium", MEDIUMSZ, 1);
  
  testread("fork large", LARGESZ, 0);
  testread("vfork large", LARGESZ, 1);
  
  // write workload - triggers COW behavior
  printf("WRITE TESTS\n");
  testwrite("fork small", SMALLSZ, 0);
  testwrite("vfork small", SMALLSZ, 1);
  
  testwrite("fork medium", MEDIUMSZ, 0);
  testwrite("vfork medium", MEDIUMSZ, 1);
  
  testwrite("fork large", LARGESZ, 0);
  testwrite("vfork large", LARGESZ, 1);
  
  printf("TESTS COMPLETED\n");

  exit(0);
}
