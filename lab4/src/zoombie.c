#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    pid_t pid = fork();
    
    if (pid == 0) {
        printf("Child %d exiting\n", getpid());
        exit(0);
    } else {
        printf("Parent %d, child %d\n", getpid(), pid);
        printf("Zombie created. Check: ps aux | grep defunct\n");
        while(1) sleep(1);
    }
    
    return 0;
}
