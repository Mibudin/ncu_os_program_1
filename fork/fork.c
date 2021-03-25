#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>


int main()
{
    pid_t current_pid;
    pid_t child_pid;
    pid_t wait_result;
    int stat_loc;

    child_pid = fork();

    current_pid = getpid();

    switch(child_pid)
    {
        // Fork error
        case -1:
            perror("[ERROR] Fork failed\n");
            exit(-1);
            break;

        // Process to test forking
        case 0:
            child_pid = fork();

            current_pid = getpid();

            switch(child_pid)
            {
                // Error occurrs
                case -1:
                    perror("[ERROR] Fork failed\n");
                    exit(-1);
                    break;

                // Child process
                case 0:
                    printf("[Child #%d] Current PID: %d, Child PID: %d\n",
                        current_pid, current_pid, child_pid);
                    printf("[Child #%d] Enter the exit code: ", current_pid);
                    scanf("%d", &stat_loc);
                    printf("[Child #%d] Exit code will be: %d\n",
                        current_pid, stat_loc);
                    exit(stat_loc);
                    break;

                // Parent process
                default:
                    wait_result = waitpid(child_pid, &stat_loc, WUNTRACED);
                    printf("[Parent #%d] Current PID: %d, Child PID: %d\n",
                        current_pid, current_pid, child_pid);
                    printf("[Parent #%d] Wait Result PID: %d, Child Exit Code: %d\n",
                        current_pid, wait_result, WEXITSTATUS(stat_loc));
                    printf("[Parent #%d] Enter the exit code: ", current_pid);
                    scanf("%d", &stat_loc);
                    printf("[Parent #%d] Exit code will be: %d\n",
                        current_pid, stat_loc);
                    exit(stat_loc);
                    break;
            }
            break;

        // Process to observe forking
        default:
            wait_result = waitpid(child_pid, &stat_loc, WUNTRACED);
            printf("[Grandparent #%d] Current PID: %d, Child PID: %d\n",
                current_pid, current_pid, child_pid);
            printf("[Grandparent #%d] Wait Result PID: %d, Child Exit Code: %d\n",
                current_pid, wait_result, WEXITSTATUS(stat_loc));
            break;
    }

    return 0;
}
