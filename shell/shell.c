#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/wait.h>
#include<sys/stat.h>


// TODO
#define TERMINAL_PROMPT "> "
// #define TERMINAL_PROMPT_LEN 2

#define SGR_DEFT       "\001\033[0m\002"          // Set all attributes to default
#define SGR_BDBR       "\001\033[1m\002"          // Set bold and bright
#define SGR_UDLN       "\001\033[4m\002"          // Set underline
#define SGR_NOUL       "\001\033[24m\002"         // Set no underline
#define SGR_CLOR(x, y) "\001\033["#x";"#y"m\002"  // Set color (x: background (40 ~ 49), y: foreground (30 ~ 39))

#define MAX_CMD_STR_LENGTH 256
#define MAX_CMD_INFO_NUM   64
#define MAX_ARG_TOK_NUM    64  // -1 (NULL)
// #define MAX_ARGS_TOKENS 32
#define MAX_CMD_REDIR_NUM  8

#define CMD_REDIR_FROM_NONE    0
#define CMD_REDIR_FROM_NRML    1
#define CMD_REDIR_TO_NONE      0
#define CMD_REDIR_TO_NRML      1
#define CMD_REDIR_TO_APPN      2
#define CMD_REDIR_TO_ERRO      3
#define CMD_REDIR_TO_ERRO_APPN 4

// #define CMD_SPLIT_DELIMETERS " \n"

#define EXIT_SUCCESS 0  // Same as the already defined one in `stdlib.h`
#define EXIT_FAILURE 1  // Same as the already defined one in `stdlib.h`
#define EXIT_ERROR   2

#define EXEC_NO_INNER -1
#define EXEC_NORMAL    0
#define EXEC_FAILURE   1
#define EXEC_PIPE      2

#define FILE_OP_FROM    O_RDONLY
#define FILE_OP_TO_TRNC O_WRONLY | O_TRUNC  | O_CREAT
#define FILE_OP_TO_APPN O_WRONLY | O_APPEND | O_CREAT
#define FILE_PERM_DEFT  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH  // For the permissions `-rw-rw-r--` of a file


// typedef struct Alias_Pair Alias_Pair;
// struct Alias_Pair
// {
//     char* alias_name;
//     char* real_name;
// };

typedef struct Command_Info Command_Info;
struct Command_Info
{
    bool has_pipe;

    int cmd_string_begin;
    int cmd_string_end;

    char** arg_strings[MAX_ARG_TOK_NUM];

    int redirect_from[MAX_CMD_REDIR_NUM];
    int redirect_to[MAX_CMD_REDIR_NUM];

    char** redirect_strings[MAX_CMD_REDIR_NUM * 2];

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

void init_cmd_info(Command_Info* cmd_info);
int set_cmd_io(Command_Info* cmd_info, int input_file_desc, int output_file_desc);

// char* alias_check(char* arg_string);

int func_cd(char** args_string);
int func_help(char** args_string);
int func_exit(char** args_string);


// const Alias_Pair alias_pairs[] =
// {
//     {"ll", "ls -alF"},
//     {"la", "ls -A"},
//     {"mv", "mv -i"},
//     {"cp", "cp -i"},
//     {"opendir", "xdg-open ."},
//     {NULL, NULL}
// };

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

bool is_term_sgr;  // Whether the terminal supports ASCII escape code.

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
    is_term_sgr = (getenv("TERM") != NULL);

    return;
}

void shell_loop()
{
    // char* cmd_string;
    // char** args_string;
    int stat_loc;
    int exec_status;

    // int current_input = STDIN_FILENO;
    int stdin_fd;
    int stdout_fd;
    int stderr_fd;

    while(true)
    {
        if(fflush(NULL) == -1)
        {
            // FIXME: ERROR: Flush all streams error.
        }
        stdin_fd = dup(STDIN_FILENO);
        stdout_fd = dup(STDOUT_FILENO);
        stderr_fd = dup(STDERR_FILENO);
        if(stdin_fd == -1 || stdout_fd == -1 || stderr_fd == -1)
        {
            // FIXME
        }

        if(isatty(STDOUT_FILENO))
        {
            // TODO
            // write(current_input, TERMINAL_PROMPT, TERMINAL_PROMPT_LEN);
            if(is_term_sgr)
            {
                fprintf(stdout, SGR_BDBR SGR_CLOR(49, 34) TERMINAL_PROMPT SGR_DEFT);
            }
            else
            {
                fprintf(stdout, TERMINAL_PROMPT);
            }
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
            // for(int j = 0; cmd_infos[i].arg_strings[j] != NULL; j++)
            // {
            //     printf(">>> %s\\\n", cmd_infos[i].arg_strings[j]);
            // }
            // for(; ;)
            if(cmd_infos[i].arg_strings[0] != NULL)
            {
                // printf(">>>>\n");
                if(fflush(NULL) == -1)
                {
                    // FIXME: ERROR: Flush all streams error.
                }
                if((exec_status = execute_inner(i)) == EXEC_NO_INNER)
                {
                    switch(exec_status = execute_outer(i))
                    {
                        case EXEC_NORMAL:
                            if(dup2(stdin_fd, STDIN_FILENO) == -1)
                            {
                                // FIXME
                            }
                            if(dup2(stdout_fd, STDOUT_FILENO) == -1)
                            {
                                // FIXME
                            }
                            if(dup2(stderr_fd, STDERR_FILENO) == -1)
                            {
                                // FIXME
                            }
                            if(fflush(NULL) == -1)
                            {
                                // FIXME
                            }
                            break;

                        case EXEC_FAILURE:
                            break;

                        case EXEC_PIPE:
                            break;
                    }
                    // TODO
                    // fprintf(stdout, ">> %d\n", WEXITSTATUS(stat_loc));
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
        //         // FIXME: //
        //     }
        // }

        // for(int i = 0; args_string[i] != NULL; i++) free(args_string[i]);
        // free(args_string);
        // free(cmd_string);

        if(close(stdin_fd) == -1)
        {
            // FIXME
        }
        if(close(stdout_fd) == -1)
        {
            // FIXME
        }
        if(close(stderr_fd) == -1)
        {
            // FIXME
        }
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
            // FIXME: ERROR: Commands too long error.
            return -1;
        }
        if(fgets(cmd_string + cmd_string_size, last_capability, stdin) == NULL)
        {
            // FIXME
            fprintf(stdout, ">>>><<<<\n");
        }
        // fprintf(stdout, ">>>>> %s\\%s\\\n", cmd_string, cmd_string + 3);

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
                if(!parsing_double_quote) parsing_single_quote = !parsing_single_quote;
            }
            else if(current_char == '\"')
            {
                if(!parsing_single_quote) parsing_double_quote = !parsing_double_quote;
            }

            cmd_string_size++;
        }

        if(feof(stdin) || ferror(stdin)) is_new_line = false;
    }

    return ++cmd_string_size;
}

int parse_commands(int cmd_string_size)
{
    // char** args_string = (char**)malloc(sizeof(char*) * MAX_ARGS_TOKENS);
    // if(args_string == NULL)
    // {
    //     // FIXME: //
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
                if(parsing_double_quote) break;
                parsing_single_quote = !parsing_single_quote;
                break;
            
            case '\"':
                if(parsing_single_quote) break;
                parsing_double_quote = !parsing_double_quote;
                break;
            
            case '\0':
                if(parsing_cmd)
                {
                    current_cmd_info->cmd_string_end = i;
                    cmd_infos_num++;
                    parsing_cmd = false;
                }
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
            
            case '|':
                if(parsing_double_quote || parsing_single_quote) break;

                cmd_string[i] = '\0';
                if(parsing_cmd)
                {
                    current_cmd_info->has_pipe = true;
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
                        // FIXME: ERROR: Commands too many error.
                    }
                    parsing_cmd = true;
                    current_cmd_info = &(cmd_infos[cmd_infos_num]);
                    init_cmd_info(current_cmd_info);
                    current_cmd_info->cmd_string_begin = i;
                }
                break;
        }
    }
    // if(parsing_cmd)
    // {
    //     current_cmd_info->cmd_string_end = i;
    //     cmd_infos_num++;
    // }

    return cmd_infos_num;
}

int parse_args(int cmd_info_index)
{
    int arg_tok_num = 0;
    Command_Info* current_cmd_info = &(cmd_infos[cmd_info_index]);
    bool parsing_token = false;
    bool parsing_single_quote = false;
    bool parsing_double_quote = false;
    bool parsing_redir_from_path = false;
    bool parsing_redir_to_path = false;
    int redir_from_num = 0;
    int redir_to_num = 0;
    for(int i = current_cmd_info->cmd_string_begin; i <= current_cmd_info->cmd_string_end; i++)
    {
        switch(cmd_string[i])
        {
            case ' ':
                if(parsing_single_quote || parsing_double_quote) break;

                cmd_string[i] = '\0';
                if(parsing_token)
                {
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
                if(parsing_double_quote) break;

                cmd_string[i] = '\0';
                if(parsing_token)
                {
                    if(parsing_single_quote)
                    {
                        arg_tok_num++;
                        parsing_token = false;
                    }
                    else
                    {
                        // FIXME: ERROR: Argument invalid error.
                    }
                }
                // else
                // {
                //     if(i + 1 <= current_cmd_info->cmd_string_end)
                //     {
                //         current_cmd_info->arg_strings[arg_tok_num] = &(cmd_string[i + 1]);
                //     }
                //     else
                //     {
                //         // FIXME: ERROR: Command parsing error.
                //     }
                // }

                parsing_single_quote = !parsing_single_quote;
                break;
            
            case '\"':
                if(parsing_single_quote) break;

                cmd_string[i] = '\0';
                if(parsing_token)
                {
                    if(parsing_double_quote)
                    {
                        arg_tok_num++;
                        parsing_token = false;
                    }
                    else
                    {
                        // FIXME: ERROR: Argument invalid error.
                    }
                }
                // else
                // {
                //     if(i + 1 <= current_cmd_info->cmd_string_end)
                //     {
                //         current_cmd_info->arg_strings[arg_tok_num] = &(cmd_string[i + 1]);
                //     }
                //     else
                //     {
                //         // FIXME: ERROR: Command parsing error.
                //     }
                // }

                parsing_double_quote = !parsing_double_quote;
                break;
            
            case '&':
                if(parsing_double_quote || parsing_single_quote) break;

                cmd_string[i] = '\0';
                if(parsing_token)
                {
                    arg_tok_num++;
                    parsing_token = false;
                }

                current_cmd_info->is_exec_background = true;
                break;

            case '<':  // TODO
                if(parsing_single_quote || parsing_double_quote) break;

                cmd_string[i] = '\0';
                if(parsing_token)
                {
                    arg_tok_num++;
                    parsing_token = false;
                }

                if(parsing_redir_from_path || parsing_redir_to_path)
                {
                    // FIXME
                }
                if(redir_from_num >= MAX_CMD_REDIR_NUM)
                {
                    // FIXME
                }
                parsing_redir_from_path = true;
                current_cmd_info->redirect_from[redir_from_num] = CMD_REDIR_FROM_NRML;
                break;
            
            case '>':  // TODO
                if(parsing_single_quote || parsing_double_quote) break;

                cmd_string[i] = '\0';
                if(parsing_token)
                {
                    arg_tok_num++;
                    parsing_token = false;
                }

                if(parsing_redir_from_path || parsing_redir_to_path)
                {
                    // FIXME
                }
                if(redir_to_num >= MAX_CMD_REDIR_NUM)
                {
                    // FIXME
                }
                parsing_redir_to_path = true;
                if(i + 1 < current_cmd_info->cmd_string_end && cmd_string[i + 1] == '>')
                {
                    cmd_string[i + 1] = '\0';
                    if(i + 2 < current_cmd_info->cmd_string_end && cmd_string[i + 2] == '&')
                    {
                        cmd_string[i + 2] = '\0';
                        current_cmd_info->redirect_to[redir_to_num] = CMD_REDIR_TO_ERRO_APPN;
                        i = i + 2;
                    }
                    else
                    {
                        printf("<><><>\n");
                        current_cmd_info->redirect_to[redir_to_num] = CMD_REDIR_TO_APPN;
                        i = i + 1;
                    }
                }
                else
                {
                    if(i + 1 < current_cmd_info->cmd_string_end && cmd_string[i + 1] == '&')
                    {
                        cmd_string[i + 1] = '\0';
                        current_cmd_info->redirect_to[redir_to_num] = CMD_REDIR_TO_ERRO;
                        i = i + 1;
                    }
                    else
                    {
                        current_cmd_info->redirect_to[redir_to_num] = CMD_REDIR_TO_NRML;
                    } 
                }
                break;

            default:
                if(!parsing_token)
                {
                    if(parsing_redir_from_path)
                    {
                        current_cmd_info->redirect_strings[redir_from_num << 1] = &(cmd_string[i]);
                        printf("><1 %d, %s\n", redir_from_num << 1, current_cmd_info->redirect_strings[redir_from_num << 1]);
                        redir_from_num++;
                        arg_tok_num--;
                        parsing_redir_from_path = false;
                    }
                    else if(parsing_redir_to_path)
                    {
                        current_cmd_info->redirect_strings[(redir_to_num << 1)+ 1] = &(cmd_string[i]);
                        printf("><2 %d, %s\n", (redir_to_num << 1) + 1, current_cmd_info->redirect_strings[(redir_to_num << 1) + 1]);
                        redir_to_num++;
                        arg_tok_num--;
                        parsing_redir_to_path = false;
                    }
                    else
                    {
                        if(arg_tok_num + 1 >= MAX_ARG_TOK_NUM)
                        {
                            // FIXME: ERROR: Arguments too many error.
                        }
                        current_cmd_info->arg_strings[arg_tok_num] = &(cmd_string[i]);
                    }

                    parsing_token = true;
                }
                break;
        }
    }
    // if(parsing_redir_from_path || parsing_redir_to_path)
    // {
    //     // FIXME
    //     printf(">>><><>< %s\\\n", current_cmd_info->redirect_strings[0]);
    // }
    current_cmd_info->arg_strings[arg_tok_num] = NULL;
    // if(arg_tok_num + 1 <= MAX_ARG_TOK_NUM)
    // {
    //     current_cmd_info->arg_strings[arg_tok_num] = NULL;
    // }
    // else
    // {
    //     // FIXME
    // }

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

    return EXEC_NO_INNER;
}

int execute_outer(int cmd_info_index)
{
    Command_Info* cmd_info = &(cmd_infos[cmd_info_index]);

    if(cmd_info->arg_strings[0] == NULL)
    {
        // FIXME
        printf(">><<>>\n");
        return -1;
    }

    int file_desc[2];
    if(cmd_info->has_pipe)
    {
        if(pipe(file_desc) == -1)
        {
            // FIXME: ERROR: Pipe failed error.
        }
    }

    pid_t exec_pid = fork();
    int stat_loc;
    int exec_status;
    
    if(exec_pid == -1)
    {
        // FIXME: ERROR: Fork failed error.
    }
    else if(exec_pid == 0)
    {
        if(cmd_info->has_pipe)
        {
            if(close(file_desc[0]) == -1)
            {
                // FIXME: ERROR: File descriptor operations error. 
                exit(EXIT_FAILURE);
            }
            set_cmd_io(cmd_info, STDIN_FILENO, file_desc[1]);  // FIXME
        }
        else
        {
            set_cmd_io(cmd_info, STDIN_FILENO, STDOUT_FILENO);  // FIXME
        }

        if(execvp(cmd_info->arg_strings[0], cmd_info->arg_strings) == -1)
        {
            // FIXME: ERROR: Command executing failed error.
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        cmd_info->exec_pid = exec_pid;

        if(!cmd_info->is_exec_background)
        {
            if(waitpid(exec_pid, &stat_loc, WUNTRACED) == -1)
            {
                // FIXME: ERROR: Wait Child failed error.
            }
        }
        cmd_info->exit_status = stat_loc;

        if(cmd_info->has_pipe)
        {
            if(cmd_info_index + 1 >= cmd_infos_num)
            {
                // FIXME
            }
            cmd_info = &(cmd_infos[cmd_info_index + 1]);
            if(close(file_desc[1]) == -1)
            {
                // FIXME
            }
            set_cmd_io(cmd_info, file_desc[0], STDOUT_FILENO);  // FIXME

            // stat_loc = execute_outer(cmd_info_index + 1);
            exec_status = EXEC_PIPE;  // TODO: Do pipe.
        }
        else
        {
            exec_status = EXEC_NORMAL;
        }
        // else
        // {
        //     set_cmd_io(cmd_info, STDIN_FILENO, STDOUT_FILENO);  // FIXME
        // }
    }

    return exec_status;
}

void init_cmd_info(Command_Info* cmd_info)
{
    cmd_info->has_pipe = false;

    cmd_info->cmd_string_begin = -1;
    cmd_info->cmd_string_end = -1;

    for(int i = 0; i < MAX_ARG_TOK_NUM; i++)
    {
        cmd_info->arg_strings[i] = NULL;
    }
    // cmd_info->arg_strings[0] = NULL;

    for(int i = 0; i < MAX_CMD_REDIR_NUM; i++)
    {
        cmd_info->redirect_from[i] = CMD_REDIR_FROM_NONE;
        cmd_info->redirect_to[i] = CMD_REDIR_TO_NONE;
        cmd_info->redirect_strings[i << 1] = NULL;
        cmd_info->redirect_strings[(i << 1) + 1] = NULL;
    }
    // cmd_info->redirect_from[0] = CMD_REDIR_FROM_NONE;
    // cmd_info->redirect_to[0] = CMD_REDIR_TO_NONE;

    // cmd_info->redirect_strings[0] = NULL;

    cmd_info->is_exec_background = false;
    cmd_info->exec_pid = 0;
    cmd_info->exit_status = EXIT_FAILURE;

    return;
}

int set_cmd_io(Command_Info* cmd_info, int input_file_desc, int output_file_desc)
{
    // TODO

    // REDIR
    int redir_from_fd, redir_from_flag, redir_from_fd2;
    for(int i = 0; i < MAX_CMD_REDIR_NUM; i++)
    {
        if(cmd_info->redirect_from[i] == CMD_REDIR_FROM_NONE) break;
        printf("<><><><>1\n");

        if(cmd_info->redirect_strings[i << 1] == NULL)
        {
            // FIXME
        }
        switch(cmd_info->redirect_from[i])
        {
            case CMD_REDIR_FROM_NRML:
                redir_from_flag = FILE_OP_FROM;
                redir_from_fd2 = STDIN_FILENO;
                break;
        }
        redir_from_fd = open(cmd_info->redirect_strings[i << 1], redir_from_flag, FILE_PERM_DEFT);
        printf("<><><><>1 %s\n", cmd_info->redirect_strings[i << 1]);
        if(redir_from_fd == NULL)
        {
            // FIXME
        }
        if(dup2(redir_from_fd, redir_from_fd2) == -1)
        {
            // FIXME
        }
        if(close(redir_from_fd) == -1)
        {
            // FIXME
        }
    }
    int redir_to_fd, redir_to_flag, redir_to_fd2;
    for(int i = 0; i < MAX_CMD_REDIR_NUM; i++)
    {
        if(cmd_info->redirect_to[i] == CMD_REDIR_TO_NONE) break;
        printf("<><><><>2\n");

        if(cmd_info->redirect_strings[(i << 1) + 1] == NULL)
        {
            // FIXME
        }
        switch(cmd_info->redirect_to[i])
        {
            case CMD_REDIR_TO_NRML:
                redir_to_flag = FILE_OP_TO_TRNC;
                redir_to_fd2 = STDOUT_FILENO;
                break;
            case CMD_REDIR_TO_APPN:
                redir_to_flag = FILE_OP_TO_APPN;
                redir_to_fd2 = STDOUT_FILENO;
                break;
            case CMD_REDIR_TO_ERRO:
                redir_to_flag = FILE_OP_TO_TRNC;
                redir_to_fd2 = STDERR_FILENO;
                break;
            case CMD_REDIR_TO_ERRO_APPN:
                redir_to_flag = FILE_OP_TO_APPN;
                redir_to_fd2 = STDERR_FILENO;
                break;
        }
        redir_to_fd = open(cmd_info->redirect_strings[(i << 1) + 1], redir_to_flag, FILE_PERM_DEFT);  // TODO
        printf("<><><><>2 %s\n", cmd_info->redirect_strings[(i << 1) + 1]);
        if(redir_to_fd == NULL)
        {
            // FIXME
        }
        if(dup2(redir_to_fd, redir_to_fd2) == -1)
        {
            // FIXME
        }
        if(close(redir_to_fd) == -1)
        {
            // FIXME
        }
    }

    // PIPE
    if(input_file_desc != STDIN_FILENO)
    {
        if(dup2(input_file_desc, STDIN_FILENO) == -1)
        {
            // FIXME
        }
        if(close(input_file_desc) == -1)
        {
            // FIXME
        }
    }
    if(output_file_desc != STDOUT_FILENO)
    {
        if(dup2(output_file_desc, STDOUT_FILENO) == -1)
        {
            // FIXME
        }
        if(close(output_file_desc) == -1)
        {
            // FIXME
        }
    }
}

// char* alias_check(char* arg_string)
// {
//     if(arg_string != NULL)
//     {
//         int alias_counter = -1;
//         while(alias_pairs[++alias_counter].alias_name != NULL)
//         {
//             if(strcmp(arg_string, alias_pairs[alias_counter].alias_name) == 0)
//             {
//                 return strdup(alias_pairs[alias_counter].real_name);
//             }
//         }
//     }

//     return arg_string;
// }

int func_cd(char** args_string)
{
    struct stat st;

    if(args_string[1] == NULL)
    {
        // FIXME: ERROR: No argument error.
        return EXIT_FAILURE;
    }

    stat(args_string[1], &st);
    if(!S_ISDIR(st.st_mode))
    {
        // FIXME: ERROR: Directory not found error.
        return EXIT_FAILURE;
    }

    if(chdir(args_string[1]) != 0)
    {
        // FIXME: ERROR: Change directory failed error.
        return EXIT_FAILURE;
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
    while(wait(NULL) != -1);
    exit(EXIT_SUCCESS);

    return EXIT_SUCCESS;
}
