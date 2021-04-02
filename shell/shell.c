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

#define ERROR_NOT_DOUND -1


typedef struct Alias_Pair Alias_Pair;
struct Alias_Pair
{
    char* alias_name;
    char* real_name;
};


void shell_init();
void shell_loop();

char* read_command();
char** split_command(char* cmd_string);
int execute_inner(char** args_string);
int execute_outer(char** args_string);

char* alias_check(char* arg_string);

int func_cd(char** args_string);
int func_help(char** args_string);
int func_exit(char** args_string);


Alias_Pair alias_pairs[] =
{
    {"ll", "ls -alF"},
    {"la", "ls -A"},
    {"mv", "mv -i"},
    {"cp", "cp -i"},
    {"opendir", "xdg-open ."},
    {NULL, NULL}
};

char *builtin_cmd[] = 
{
    "cd",
    "help",
    "exit",
    NULL
};

int (*builtin_func[])(char** args_string) =
{
    &func_cd,
    &func_help,
    &func_exit
};


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
            // FIXME
        }

        if(isatty(current_input))
        {
            // FIXME
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
            if(execute_inner(args_string) == ERROR_NOT_DOUND)
            {
                stat_loc = execute_outer(args_string);
                // TODO
                printf(">> %d\n", WEXITSTATUS(stat_loc));
            }
            else
            {
                // FIXME
            }
        }

        for(int i = 0; args_string[i] != NULL; i++) free(args_string[i]);
        free(args_string);
        free(cmd_string);
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
        // FIXME
        return args_string;
    }

    char* token = strtok(cmd_string, CMD_SPLIT_DELIMETERS);
    int token_counter = 0;
    char* token_alias = alias_check(token);
    if(token != NULL && token_alias != token)
    {
        token = strtok(token_alias, CMD_SPLIT_DELIMETERS);
        while(token != NULL && token_counter < MAX_ARGS_TOKENS)
        {
            args_string[token_counter++] = strdup(token);
            token = strtok(NULL, CMD_SPLIT_DELIMETERS);
        }
        free(token_alias);

        strtok(cmd_string, CMD_SPLIT_DELIMETERS);
        token = strtok(NULL, CMD_SPLIT_DELIMETERS);
    }

    while(token != NULL && token_counter < MAX_ARGS_TOKENS)
    {
        args_string[token_counter++] = strdup(token);
        token = strtok(NULL, CMD_SPLIT_DELIMETERS);
    }
    args_string[token_counter] = token;

    return args_string;
}

int execute_inner(char** args_string)
{
    int builtin_counter = -1;
    while(builtin_cmd[++builtin_counter] != NULL)
    {
        if(strcmp(args_string[0], builtin_cmd[builtin_counter]) == 0)
        {
            return (*builtin_func[builtin_counter])(args_string);
        }
    }

    // FIXME
    return ERROR_NOT_DOUND;
}

int execute_outer(char** args_string)
{
    pid_t exec_pid = fork();
    int stat_loc;
    
    if(exec_pid == -1)
    {
        // FIXME
    }
    else if(exec_pid == 0)
    {
        if(execvp(args_string[0], args_string) == -1)
        {
            // FIXME
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        waitpid(exec_pid, &stat_loc, WUNTRACED);  // FIXME
    }

    return stat_loc;
}

char* alias_check(char* arg_string)
{
    int alias_counter = -1;
    while(alias_pairs[++alias_counter].alias_name != NULL)
    {
        if(strcmp(arg_string, alias_pairs[alias_counter].alias_name) == 0)
        {
            return strdup(alias_pairs[alias_counter].real_name);
        }
    }

    return arg_string;
}

int func_cd(char** args_string)
{
    if(args_string[1] != NULL)
    {
        // FIXME
    }

    if(chdir(args_string[1]) != 0)
    {
        // FIXME
    }

    return EXIT_SUCCESS;
}

int func_help(char** args_string)
{
    // TODO

    return EXIT_SUCCESS;
}

int func_exit(char** args_string)
{
    exit(EXIT_SUCCESS);

    return EXIT_SUCCESS;
}
