#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

volatile sig_atomic_t timeout_reached = 0;

void timeout_handler(int sig) {  
    timeout_reached = 1;
}

int main(int argc, char **argv) {
  int seed = -1;
  int array_size = -1;
  int pnum = -1;
  bool with_files = false;
  int timeout = 0;

  int (*pipes)[2] = NULL;
  char **filenames = NULL;
  pid_t *child_pids = NULL;

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {"by_files", no_argument, 0, 'f'},
                                      {"timeout", required_argument, 0, 't'},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "ft:", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            seed = atoi(optarg);
            if (seed <= 0){
              printf("seed must be positive number\n");
              return 1;
            }
            break;
          case 1:
            array_size = atoi(optarg);
            if (array_size <= 0){
              printf("array_size must be positive number\n");
              return 1;
            }
            break;
          case 2:
            pnum = atoi(optarg);
            if (pnum <= 0){
              printf("pnum must be positive number\n");
              return 1;
            }
            break;
          case 3:
            with_files = true;
            break;
          case 4: 
            timeout = atoi(optarg);
            if (timeout <= 0) {
              printf("timeout must be positive number\n");
              return 1;
            }
            break;
          default:  
            printf("Index %d is out of options\n", option_index);
        }
        break;
      case 'f':
        with_files = true;
        break;
      
      case 't':
        timeout = atoi(optarg);
        if (timeout <= 0) {
          printf("timeout must be positive number\n");
          return 1;
        }
        break;

      case '?':
        break;

      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }

  if (optind < argc) {
    printf("Has at least one no option argument\n");
    return 1;
  }

  if (seed == -1 || array_size == -1 || pnum == -1) {
    printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" [--timeout \"sec\"]\n",
           argv[0]);
    return 1;
  }

  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);
  int active_child_processes = 0;

  child_pids = malloc(pnum * sizeof(pid_t));

  if (with_files) {
    filenames = malloc(pnum * sizeof(char*));
    for (int i = 0; i < pnum; i++) {
      filenames[i] = malloc(256);
      snprintf(filenames[i], 256, "min_max_%d.txt", i);
    }
  } else {
    pipes = malloc(pnum * sizeof(int[2]));
    for (int i = 0; i < pnum; i++) {
      if (pipe(pipes[i]) < 0) {
        printf("Failed to create pipe for process %d\n", i);
        return 1;
      }
    }
  }

  if (timeout > 0) {
    signal(SIGALRM, timeout_handler);
    alarm(timeout);
  }

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  for (int i = 0; i < pnum; i++) {
    pid_t child_pid = fork();
    if (child_pid >= 0) {
      // successful fork
      active_child_processes += 1;
      if (child_pid == 0) {
        // child process
        unsigned int begin = i * (array_size / pnum);
        unsigned int end = (i == pnum - 1) ? array_size : (i + 1) * (array_size / pnum);
        
        struct MinMax local_min_max = GetMinMax(array, begin, end);
        
        if (with_files) {
          // use files here
          FILE* file = fopen(filenames[i], "w");
          fprintf(file, "%d %d", local_min_max.min, local_min_max.max);
          fclose(file);
        } else {
          close(pipes[i][0]);
          write(pipes[i][1], &local_min_max.min, sizeof(int));
          write(pipes[i][1], &local_min_max.max, sizeof(int));
          close(pipes[i][1]);
        }
        free(array);
        exit(0);
      } else {
        child_pids[i] = child_pid; 
      }

    } else {
      printf("Fork failed!\n");
      return 1;
    }
  }

  if (!with_files) {
    for (int i = 0; i < pnum; i++) {
      close(pipes[i][1]); 
    }
  }

  while (active_child_processes > 0) {
    if (timeout_reached) {
      printf("Timeout reached! Killing child processes...\n");
      for (int i = 0; i < pnum; i++) {
        if (child_pids[i] > 0) {
          kill(child_pids[i], SIGKILL);
        }
      }
      break;
    }
    
    pid_t finished_pid = waitpid(-1, NULL, WNOHANG);
    if (finished_pid > 0) {
      active_child_processes -= 1;
      for (int i = 0; i < pnum; i++) {
        if (child_pids[i] == finished_pid) {
          child_pids[i] = 0;
          break;
        }
      }
    } else if (finished_pid == 0) {
      usleep(100000);
    }
  }

  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  for (int i = 0; i < pnum; i++) {
    if (timeout_reached && child_pids[i] != 0) continue;

    int min = INT_MAX;
    int max = INT_MIN;

    if (with_files) {
      // read from files
      FILE* file = fopen(filenames[i], "r");
      if (file != NULL) {
        fscanf(file, "%d %d", &min, &max);
        fclose(file);
        remove(filenames[i]);
      }
    } else {
      // read from pipes
      read(pipes[i][0], &min, sizeof(int));
      read(pipes[i][0], &max, sizeof(int));
      close(pipes[i][0]);
    }

    if (min < min_max.min) min_max.min = min;
    if (max > min_max.max) min_max.max = max;
  }

  if (with_files) {
    for (int i = 0; i < pnum; i++) {
      free(filenames[i]);
    }
    free(filenames);
  } else {
    free(pipes);
  }

  free(child_pids); 

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(array);

  printf("Min: %d\n", min_max.min);
  printf("Max: %d\n", min_max.max);
  printf("Elapsed time: %fms\n", elapsed_time);

  if (timeout_reached) {
    printf("Warning: Some processes were terminated due to timeout\n");
  }
  
  fflush(NULL);
  return 0;
}