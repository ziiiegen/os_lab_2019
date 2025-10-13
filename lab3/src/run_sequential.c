#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <seed> <array_size>\n", argv[0]);
        return 1;
    }
    
    pid_t pid = fork();
    
    if (pid == 0) {
        // dother
        char *args[] = {"./sequential_min_max", argv[1], argv[2], NULL};
        execvp(args[0], args);
        perror("execvp failed");
        return 1;
    } else {
        // parrent 
        wait(NULL);
        printf("sequential_min_max finished\n");
    }
    
    return 0;
}