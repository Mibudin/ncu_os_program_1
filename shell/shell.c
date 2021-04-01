#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include<unistd.h>
#include<sys/wait.h>


#define TERMINAL_PROMPT "> "
#define TERMINAL_PROMPT_LEN 2

#define MAX_ARGS_TOKENS 32

#define CMD_SPLIT_DELIMETERS " \n"

// #define EXIT_SUCCESS 0  // Already defined in `stdlib.h`
// #define EXIT_FAILURE 1  // Already defined in `stdlib.h`
#define EXIT_ERROR   2


void shell_init();
void shell_loop();

char* read_command();
char** split_command(char* cmd_string);
int execute_command(char** args_string);


int main()
{
    shell_init();

    shell_loop();

    return EXIT_SUCCESS;
}

void shell_init()
{
    return;
}

void shell_loop()
{
    char* cmd_string;
    char** args_string;
    int stat_loc;

    int current_input = STDIN_FILENO;

    while(true)
    {
        if(fflush(NULL) == -1)
        {
            // TODO
        }

        if(isatty(current_input))
        {
            write(current_input, TERMINAL_PROMPT, TERMINAL_PROMPT_LEN);
        }

        // READ
        cmd_string = read_command();
        // write(current_input, cmd_string, strlen(cmd_string));

        // PARSE
        args_string = split_command(cmd_string);

        // SEARCH
        // EXCUTE: fork inside
        if(args_string[0] != NULL)
        {
            stat_loc = execute_command(args_string);
            printf(">> %d\n", WEXITSTATUS(stat_loc));
        }

        free(cmd_string);
        free(args_string);
    }

    return;
}

char* read_command()
{
    char* cmd_string = NULL;
    size_t buffer_size = 0;
    getline(&cmd_string, &buffer_size, stdin);

    return cmd_string;
}

char** split_command(char* cmd_string)
{
    char** args_string = (char**)malloc(sizeof(char*) * MAX_ARGS_TOKENS);
    if(args_string == NULL)
    {
        // TODO
        return args_string;
    }

    char* token = strtok(cmd_string, CMD_SPLIT_DELIMETERS);
    int counter = 0;
    while(token != NULL && counter < MAX_ARGS_TOKENS)
    {
        args_string[counter++] = strdup(token);
        token = strtok(NULL, CMD_SPLIT_DELIMETERS);
    }
    args_string[counter] = token;

    return args_string;
}

int execute_command(char** args_string)
{
    pid_t exec_pid = fork();
    int stat_loc;
    
    if(exec_pid == -1)
    {
        // TODO
    }
    else if(exec_pid == 0)
    {
        if(execvp(args_string[0], args_string) == -1)
        {
            // TODO
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        waitpid(exec_pid, &stat_loc, WUNTRACED);  // TODO
    }

    return stat_loc;
}
