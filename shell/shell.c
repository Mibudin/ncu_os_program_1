#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/stat.h>


#define TERMINAL_PROMPT "> "
#define TERMINAL_PROMPT_LEN 2

#define MAX_CMD_STR_LENGTH   256
#define MAX_CMD_INFO_NUM     64
#define MAX_ARG_TOK_NUM      64  // -1 (NULL)
#define MAX_ARGS_TOKENS 32

#define CMD_SPLIT_DELIMETERS " \n"

#define EXIT_SUCCESS 0  // Same as the already defined one in `stdlib.h`
#define EXIT_FAILURE 1  // Same as the already defined one in `stdlib.h`
#define EXIT_ERROR   2

#define ERROR_NOT_DOUND -1


typedef struct Alias_Pair Alias_Pair;
struct Alias_Pair
{
    char* alias_name;
    char* real_name;
};

typedef struct Command_Info Command_Info;
struct Command_Info
{
    Command_Info* next;

    int cmd_string_begin;
    int cmd_string_end;

    char** arg_strings[MAX_ARG_TOK_NUM];

    int redirect_from;
    int redirect_in;

    bool is_exec_background;
    pid_t exec_pid;
    int exit_status;
};


void shell_init();
void shell_loop();

int read_commands();
int parse_commands(int cmd_string_size);
int parse_args(int cmd_info_index);
int execute_inner(int cmd_info_index);
int execute_outer(int cmd_info_index);

char* alias_check(char* arg_string);

int func_cd(char** args_string);
int func_help(char** args_string);
int func_exit(char** args_string);


const Alias_Pair alias_pairs[] =
{
    {"ll", "ls -alF"},
    {"la", "ls -A"},
    {"mv", "mv -i"},
    {"cp", "cp -i"},
    {"opendir", "xdg-open ."},
    {NULL, NULL}
};

const char* builtin_cmd[] = 
{
    "cd",
    "help",
    "exit",
    NULL
};
const int (*builtin_func[])(char** args_string) =
{
    &func_cd,
    &func_help,
    &func_exit
};

char cmd_string[MAX_CMD_STR_LENGTH];
Command_Info cmd_infos[MAX_CMD_INFO_NUM];
int cmd_infos_num;


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
    // char* cmd_string;
    // char** args_string;
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
        // cmd_string = read_command(); // TODO
        // write(current_input, cmd_string, strlen(cmd_string));
        int n = read_commands();

        // PARSE
        // args_string = split_command(cmd_string);
        parse_commands(n);
        for(int i = 0; i < cmd_infos_num; i++)
        {
            parse_args(i);
            for(int j = 0; cmd_infos[i].arg_strings[j] != NULL; j++)
            {
                printf(">>> %s\\\n", cmd_infos[i].arg_strings[j]);
            }
            if(cmd_infos[i].arg_strings[0] != NULL)
            {
                if(execute_inner(i) == ERROR_NOT_DOUND)
                {
                    stat_loc = execute_outer(i);
                    // TODO
                    fprintf(stdout, ">> %d\n", WEXITSTATUS(stat_loc));
                }
            }
        }

        // SEARCH
        // EXCUTE: fork inside
        // if(args_string[0] != NULL)
        // if()
        // {
        //     if(execute_inner(args_string) == ERROR_NOT_DOUND)
        //     {
        //         stat_loc = execute_outer(args_string);
        //         // TODO
        //         printf(">> %d\n", WEXITSTATUS(stat_loc));
        //     }
        //     else
        //     {
        //         // FIXME
        //     }
        // }

        // for(int i = 0; args_string[i] != NULL; i++) free(args_string[i]);
        // free(args_string);
        // free(cmd_string);
    }

    return;
}

int read_commands()
{
    // char* cmd_string = NULL;
    // size_t buffer_size = 0;
    // getline(&cmd_string, &buffer_size, stdin);

    // return cmd_string;

    int cmd_string_size = 0;
    int last_capability;
    bool is_new_line = true;
    bool parsing_single_quote = false;
    bool parsing_double_quote = false;
    char current_char;
    while(is_new_line)
    {
        // TODO `<<`

        last_capability = MAX_CMD_STR_LENGTH - cmd_string_size;
        if(last_capability <= 0)
        {
            // FIXME
            return -1;
        }
        fgets(cmd_string + cmd_string_size, last_capability, stdin);

        is_new_line = false;
        while(true)
        {
            current_char = cmd_string[cmd_string_size];
            if(current_char                    == '\\' &&
               cmd_string[cmd_string_size + 1] == '\n')
            {
                is_new_line = true;
                break;
            }
            else if(current_char == '\n')
            {
                if(parsing_single_quote || parsing_double_quote)
                {
                    is_new_line = true;
                    cmd_string_size++;
                }
                break;
            }
            else if(current_char == '\'')
            {
                parsing_single_quote = !parsing_single_quote;
            }
            else if(current_char == '\"')
            {
                parsing_double_quote = !parsing_double_quote;
            }

            cmd_string_size++;
        }
    }

    return cmd_string_size;
}

int parse_commands(int cmd_string_size)
{
    // char** args_string = (char**)malloc(sizeof(char*) * MAX_ARGS_TOKENS);
    // if(args_string == NULL)
    // {
    //     // FIXME
    //     return args_string;
    // }

    // char* token = strtok(cmd_string, CMD_SPLIT_DELIMETERS);
    // int token_counter = 0;
    // char* token_alias = alias_check(token);
    // if(token != NULL && token_alias != token)
    // {
    //     token = strtok(token_alias, CMD_SPLIT_DELIMETERS);
    //     while(token != NULL && token_counter < MAX_ARGS_TOKENS)
    //     {
    //         args_string[token_counter++] = strdup(token);
    //         token = strtok( NULL, CMD_SPLIT_DELIMETERS);
    //     }
    //     free(token_alias);

    //     strtok(cmd_string, CMD_SPLIT_DELIMETERS);
    //     token = strtok(NULL, CMD_SPLIT_DELIMETERS);
    // }

    // while(token != NULL && token_counter < MAX_ARGS_TOKENS)
    // {
    //     args_string[token_counter++] = strdup(token);
    //     token = strtok(NULL, CMD_SPLIT_DELIMETERS);
    // }
    // args_string[token_counter] = token;

    // return args_string;

    cmd_infos_num = 0;
    Command_Info* current_cmd_info;
    bool parsing_cmd = false;
    bool parsing_single_quote = false;
    bool parsing_double_quote = false;
    int i;
    for(i = 0; i <= cmd_string_size; i++)
    {
        switch(cmd_string[i])
        {
            case ' ':
                break;

            case '\t':
                if(parsing_double_quote || parsing_single_quote) break;

                cmd_string[i--] = ' ';
                break;

            case '\n':
                if(parsing_double_quote || parsing_single_quote) break;

                cmd_string[i--] = ' ';
                break;
            
            case '\'':
                parsing_single_quote = !parsing_single_quote;
                break;
            
            case '\"':
                parsing_double_quote = !parsing_double_quote;
                break;

            case ';':
                if(parsing_double_quote || parsing_single_quote) break;

                cmd_string[i] = '\0';
                if(parsing_cmd)
                {
                    current_cmd_info->cmd_string_end = i;
                    cmd_infos_num++;
                    parsing_cmd = false;
                }
                break;

            case '&':
                if(parsing_double_quote || parsing_single_quote) break;

                cmd_string[i] = '\0';
                if(parsing_cmd)
                {
                    current_cmd_info->is_exec_background = true;
                    current_cmd_info->cmd_string_end = i;
                    cmd_infos_num++;
                    parsing_cmd = false;
                }
                break;

            default:
                if(!parsing_cmd)
                {
                    if(cmd_infos_num + 1 > MAX_CMD_INFO_NUM)
                    {
                        // FIXME
                    }
                    parsing_cmd = true;
                    current_cmd_info = &(cmd_infos[cmd_infos_num]);
                    current_cmd_info->cmd_string_begin = i;
                }
                break;
        }
    }
    if(parsing_cmd)
    {
        current_cmd_info->cmd_string_end = i;
        cmd_infos_num++;
    }

    return cmd_infos_num;
}

int parse_args(int cmd_info_index)
{
    int arg_tok_num = 0;
    Command_Info* current_cmd_info = &(cmd_infos[cmd_info_index]);
    char** current_arg_string;
    bool parsing_token = false;
    bool parsing_single_quote = false;
    bool parsing_double_quote = false;
    bool parsing_file_path = false;
    for(int i = current_cmd_info->cmd_string_begin; i <= current_cmd_info->cmd_string_end; i++)
    {
        // current_char = cmd_string[i];
        switch(cmd_string[i])
        {
            case ' ':
                if(parsing_single_quote || parsing_double_quote) break;

                if(parsing_token)
                {
                    cmd_string[i] = '\0';
                    arg_tok_num++;
                    parsing_token = false;
                }
                break;

            case '\0':
                if(parsing_token)
                {
                    arg_tok_num++;
                    parsing_token = false;
                }
                break;
            
            case '\'':
                if(parsing_token)
                {
                    cmd_string[i] = '\0';
                    if(!parsing_single_quote)
                    {
                        if(i + 1 < current_cmd_info->cmd_string_end)
                        {
                            current_cmd_info->arg_strings[arg_tok_num] = &(cmd_string[i + 1]);
                        }
                        else
                        {
                            // FIXME
                        }
                    }
                    else
                    {
                        arg_tok_num++;
                        parsing_token = false;
                    }
                }

                parsing_single_quote = !parsing_single_quote;
                break;
            
            case '\"':
                if(parsing_token)
                {
                    cmd_string[i] = '\0';
                    if(!parsing_double_quote)
                    {
                        if(i + 1 < current_cmd_info->cmd_string_end)
                        {
                            current_cmd_info->arg_strings[arg_tok_num] = &(cmd_string[i + 1]);
                        }
                        else
                        {
                            // FIXME
                        }
                    }
                    else
                    {
                        arg_tok_num++;
                        parsing_token = false;
                    }
                }

                parsing_double_quote = !parsing_double_quote;
                break;
            
            // case '|':  // TODO
            //     if(parsing_token)
            //     {
            //         arg_tok_num++;
            //         parsing_token = false;
            //     }
            //     break;

            // case '>':  // TODO
            //     if(parsing_token)
            //     {
            //         arg_tok_num++;
            //         parsing_token = false;
            //     }
            //     break;

            // case '<':  // TODO
            //     if(parsing_token)
            //     {
            //         arg_tok_num++;
            //         parsing_token = false;
            //     }

            //     if(i + 1 < current_cmd_info->cmd_string_end)
            //     {
            //         if(cmd_string[i + 1] == '<')
            //         {

            //         }
            //     }
            //     break;

            default:
                if(!parsing_token)
                {
                    if(arg_tok_num + 1 >= MAX_ARG_TOK_NUM)
                    {
                        // FIXME
                    }
                    parsing_token = true;
                    current_cmd_info->arg_strings[arg_tok_num] = &(cmd_string[i]);
                }
                break;
        }
    }
    if(arg_tok_num + 1 <= MAX_ARG_TOK_NUM)
    {
        current_cmd_info->arg_strings[arg_tok_num] = NULL;
    }
    else
    {
        // FIXME
    }

    return arg_tok_num;
}

int execute_inner(int cmd_info_index)
{
    Command_Info* cmd_info = &(cmd_infos[cmd_info_index]);
    if(cmd_info->arg_strings[0] != NULL)
    {
        int builtin_counter = -1;
        while(builtin_cmd[++builtin_counter] != NULL)
        {
            if(strcmp(cmd_info->arg_strings[0], builtin_cmd[builtin_counter]) == 0)
            {
                return (*builtin_func[builtin_counter])(cmd_info->arg_strings);
            }
        }
    }

    // FIXME
    return ERROR_NOT_DOUND;
}

int execute_outer(int cmd_info_index)
{
    Command_Info* cmd_info = &(cmd_infos[cmd_info_index]);

    pid_t exec_pid = fork();
    int stat_loc;
    
    if(exec_pid == -1)
    {
        // FIXME
    }
    else if(exec_pid == 0)
    {
        if(execvp(cmd_info->arg_strings[0], cmd_info->arg_strings) == -1)
        {
            // FIXME
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        if(!cmd_info->is_exec_background)
        {
            waitpid(exec_pid, &stat_loc, WUNTRACED);  // FIXME
        }
    }

    return stat_loc;
}

char* alias_check(char* arg_string)
{
    if(arg_string != NULL)
    {
        int alias_counter = -1;
        while(alias_pairs[++alias_counter].alias_name != NULL)
        {
            if(strcmp(arg_string, alias_pairs[alias_counter].alias_name) == 0)
            {
                return strdup(alias_pairs[alias_counter].real_name);
            }
        }
    }

    return arg_string;
}

int func_cd(char** args_string)
{
    struct stat st;

    if(args_string[1] == NULL)
    {
        // FIXME
        return EXIT_FAILURE;
    }

    stat(args_string[1], &st);
    if(!S_ISDIR(st.st_mode))
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
    fprintf(stdout, "                                \n"
                    "/******************************\\\n"
                    "|                              |\n"
                    "|      Nice to meet you.       |\n"
                    "|                              |\n"
                    "|    Welcome to this shell.    |\n"
                    "|                              |\n"
                    "\\***********#******#***********/\n");
    fprintf(stdout, "           /        \\           \n"
                    "          /          \\          \n"
                    " v       /            \\       v \n"
                    "*#~~~~~~#~~~~~~~~~~~~~~#~~~~~~#*\n"
                    " { -  -  -  -  -  -  -  -  -  } \n"
                    " {  THE ETERNAL DEVELOPING! - } \n"
                    " { - THE EVERLASTING BUGS! -  } \n"
                    " {  - THE PERMANENT PAIN!-  - } \n"
                    " { -  -THE FOREVER WORK!-  -  } \n"
                    " {  -  -  -  -  -  -  -  -  - } \n"
                    "*#~~~~~~~~~~~~~~~~~~~~~~~~~~~~#*\n"
                    " ^                            ^ \n"
                    "                                \n");

    return EXIT_SUCCESS;
}

int func_exit(char** args_string)
{
    wait(NULL);
    exit(EXIT_SUCCESS);

    return EXIT_SUCCESS;
}
