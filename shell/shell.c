#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/wait.h>
#include<sys/stat.h>


#define SGR_DEFT          "\001\033[0m\002"          // Set all attributes to be default.
#define SGR_BDBR          "\001\033[1m\002"          // Set to be bold and bright.
#define SGR_NOBB          "\001\033[22m\002"         // Set to be not bold and bright.
#define SGR_UDLN          "\001\033[4m\002"          // Set to be with underline.
#define SGR_NOUL          "\001\033[24m\002"         // Set to be with no underline.
#define SGR_CLOR(b, f)    "\001\033["#b";"#f"m\002"  // Set color (b: background (40 ~ 49), f: foreground (30 ~ 39)).

#define TERM_PROMPT       "\n> "
#define TERM_EXIT         "\n>> %d <<\n\n"           // With a `%d` as the exit code.
#define TERM_PROMPT_SGR   SGR_BDBR SGR_CLOR(49, 34) TERM_PROMPT SGR_DEFT
#define TERM_EXIT_SGR     SGR_BDBR SGR_CLOR(49, 32) TERM_EXIT   SGR_DEFT
#define TERM_EXIT_ERR_SGR SGR_BDBR SGR_CLOR(49, 31) TERM_EXIT   SGR_DEFT
#define TERM_ERR          "\n>>> ERROR <<<\n"    \
                            ">-- Type: %s --<\n" \
                            ">--  Msg: %s --<\n" \
                            ">--  Pos: %s --<\n\n"   // With `%s` for the type, `%s` for the message, `%s` for the position.
#define TERM_ERR_SGR      SGR_BDBR SGR_CLOR(49, 31) "\n>>> ERROR <<<\n"    \
                          SGR_NOBB SGR_CLOR(49, 33)   ">-- Type: %s --<\n" \
                                                      ">--  Msg: %s --<\n" \
                                                      ">--  Pos: %s --<\n\n" SGR_DEFT

#define MAX_CMD_STR_LENGTH     256
#define MAX_CMD_INFO_NUM        64
#define MAX_ARG_TOK_NUM         64  // The last argument token are should be `NULL`.
#define MAX_CMD_REDIR_NUM        8

#define CMD_REDIR_FROM_NONE      0
#define CMD_REDIR_FROM_NRML      1
#define CMD_REDIR_TO_NONE        0
#define CMD_REDIR_TO_NRML        1
#define CMD_REDIR_TO_APPN        2
#define CMD_REDIR_TO_ERRO        3
#define CMD_REDIR_TO_ERRO_APPN   4

#define EXIT_SUCCESS             0  // Same as the already defined one in `stdlib.h`
#define EXIT_FAILURE             1  // Same as the already defined one in `stdlib.h`
#define EXIT_ERROR               2
#define EXIT_BACKGROUND          0

#define EXEC_EMPTY              -3
#define EXEC_NO_INNER           -2
#define EXEC_NOT_YET            -1
#define EXEC_NORMAL              0
#define EXEC_FAILURE             1
#define EXEC_PIPE                2
#define EXEC_PIPE_FAILURE        3

#define SET_IO_SUCCESS           0
#define SET_IO_FAILURE           1

#define FILE_OP_FROM    O_RDONLY
#define FILE_OP_TO_TRNC O_WRONLY | O_TRUNC  | O_CREAT
#define FILE_OP_TO_APPN O_WRONLY | O_APPEND | O_CREAT
#define FILE_PERM_DEFT  S_IRUSR  | S_IWUSR  | \
                        S_IRGRP  | S_IWGRP  | S_IROTH  // The permissions `-rw-rw-r--` of a file

#define ERR_UNK   -1  // Unknown Error (and other unlisted error code)
#define ERR_NONE   0  // None Error
#define ERR_CMPLX  1  // Complex Error
#define ERR_IO     2  // I/O Error
#define ERR_PIPE   3  // Pipe Error
#define ERR_FORK   4  // Fork Error
#define ERR_WAIT   5  // Wait Error
#define ERR_RDCMD  6  // Read Commands Error
#define ERR_PRCMD  7  // Parse Commands Error
#define ERR_PRARG  8  // Parse Arguments Error
#define ERR_EXCMD  9  // Execute Command Error
#define ERR_CMDLG 10  // Commands Too Long Error
#define ERR_CMDMN 11  // Commands Too Many Error
#define ERR_ARGIV 12  // Argument Invalid Error
#define ERR_ARGMN 13  // Arguments Too Many Error
#define ERR_RDRIV 14  // Redirection Argument Invalid Error
#define ERR_RDRMN 15  // Redirection Too Many Error
#define ERR_CMDIN 16  // Command Inner Error
#define ERR_CMDIO 17  // Command I/O Error
#define ERR_CMDEP 18  // Command Empty Error
#define ERR_CMDEX 19  // Command Executing Error
#define ERR_CMDPP 20  // Command Pipe Error
#define ERR_CDNAR 21  // CD: No Argument Error
#define ERR_CDNDR 22  // CD: Not Directory Error
#define ERR_CDFAL 23  // CD: Failure Error

#define FNA       __func__  // The name string of the current function.


typedef struct Command_Info Command_Info;
struct Command_Info
{
    bool has_pipe;

    int cmd_string_begin;
    int cmd_string_end;

    char** arg_strings[MAX_ARG_TOK_NUM];

    int redirect_from[MAX_CMD_REDIR_NUM];
    int redirect_to  [MAX_CMD_REDIR_NUM];

    char** redirect_strings[MAX_CMD_REDIR_NUM * 2];

    bool  is_exec_background;
    pid_t exec_pid;
    int   exit_status;
};


void shell_init();
int  shell_loop();

int  read_commands();

int  parse_commands(const int cmd_string_size);
int  parse_args(const int cmd_info_index);

int  execute_inner(const int cmd_info_index);
int  execute_outer(const int cmd_info_index);

void init_cmd_info(Command_Info* cmd_info);
int  set_cmd_io(const Command_Info* cmd_info, const int input_file_desc, const int output_file_desc);

int  func_cd  (const char** args_string);
int  func_help(const char** args_string);
int  func_exit(const char** args_string);

int  error(const int error_type, const char* position);


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

int stdin_fd;
int stdout_fd;
int stderr_fd;


int main()
{
    shell_init();

    int exit_code = shell_loop();

    return exit_code;
}

void shell_init()
{
    is_term_sgr = (getenv("TERM") != NULL);

    return;
}

int shell_loop()
{
    int cmd_string_size;

    Command_Info* cmd_info;
    int exec_status;
    int exit_code;

    bool should_restart_loop;

    while(true)
    {
        should_restart_loop = false;

        if(fflush(NULL) == -1)
        {
            // /FIXME: ERROR: Flush all streams error.
            error(ERR_IO, FNA);
            return EXIT_FAILURE;
        }

        stdin_fd = dup(STDIN_FILENO);
        stdout_fd = dup(STDOUT_FILENO);
        stderr_fd = dup(STDERR_FILENO);
        if(stdin_fd == -1 || stdout_fd == -1 || stderr_fd == -1)
        {
            // /FIXME: ERROR: File streams operations error.
            error(ERR_IO, FNA);
            return EXIT_FAILURE;
        }

        if(isatty(STDOUT_FILENO))
        {
            if(is_term_sgr)
            {
                fprintf(stdout, TERM_PROMPT_SGR);
            }
            else
            {
                fprintf(stdout, TERM_PROMPT);
            }
        }

        // READ
        if((cmd_string_size = read_commands()) == -1)
        {
            // /FIXME: ERROR: Command input error.
            error(ERR_RDCMD, FNA);
            continue;
        }

        // PARSE
        if(parse_commands(cmd_string_size) == -1)
        {
            // /FIXME: ERROR: Commands parsing error.
            error(ERR_PRCMD, FNA);
            continue;
        }
        for(int i = 0; i < cmd_infos_num; i++)
        {
            if(parse_args(i) == -1)
            {
                // /FIXME: ERROR: Command arguments parsing error.
                error(ERR_PRARG, FNA);
                should_restart_loop = true;
            }
        }
        if(should_restart_loop) continue;

        // EXECUTE
        exec_status = EXEC_NOT_YET;
        for(int i = 0; i < cmd_infos_num && exec_status != EXEC_FAILURE; i++)
        {
            cmd_info = &(cmd_infos[i]);
            if(cmd_info->arg_strings[0] != NULL)
            {
                if(fflush(NULL) == -1)
                {
                    // /FIXME: ERROR: Flush all streams error.
                    error(ERR_IO, FNA);
                    return EXIT_FAILURE;
                }

                if((exec_status = execute_inner(i)) == EXEC_NO_INNER)
                {
                    exec_status = execute_outer(i);
                }

                switch(exec_status)
                {
                    case EXEC_FAILURE:
                    case EXEC_PIPE_FAILURE:
                        // /FIXME: ERROR: Command executing error.
                        error(ERR_EXCMD, FNA);
                        for(i++; i < cmd_infos_num && cmd_infos[i].has_pipe == true; i++);

                    case EXEC_EMPTY:
                    case EXEC_NORMAL:
                        if(dup2(stdin_fd, STDIN_FILENO) == -1)
                        {
                            // /FIXME: ERROR: File streams operations error.
                            error(ERR_IO, FNA);
                            return EXIT_FAILURE;
                        }
                        if(dup2(stdout_fd, STDOUT_FILENO) == -1)
                        {
                            // /FIXME: ERROR: File streams operations error.
                            error(ERR_IO, FNA);
                            return EXIT_FAILURE;
                        }
                        if(dup2(stderr_fd, STDERR_FILENO) == -1)
                        {
                            // /FIXME: ERROR: File streams operations error.
                            error(ERR_IO, FNA);
                            return EXIT_FAILURE;
                        }
                        if(fflush(NULL) == -1)
                        {
                            // /FIXME: ERROR: Flush all streams error.
                            error(ERR_IO, FNA);
                            return EXIT_FAILURE;
                        }

                        if(exec_status == EXEC_FAILURE) break;

                        exit_code = WEXITSTATUS(cmd_info->exit_status);
                        if(is_term_sgr)
                        {
                            if(exit_code == EXIT_SUCCESS)
                            {
                                fprintf(stdout, TERM_EXIT_SGR, exit_code);
                            }
                            else
                            {
                                fprintf(stdout, TERM_EXIT_ERR_SGR, exit_code);
                            }
                        }
                        else
                        {
                            fprintf(stdout, TERM_EXIT, exit_code);
                        }
                        break;

                    case EXEC_PIPE:
                        break;
                }
            }
        }

        if(close(stdin_fd) == -1)
        {
            // /FIXME: ERROR: File streams error.
            error(ERR_IO, FNA);
            return EXIT_FAILURE;
        }
        if(close(stdout_fd) == -1)
        {
            // /FIXME: ERROR: File streams error.
            error(ERR_IO, FNA);
            return EXIT_FAILURE;
        }
        if(close(stderr_fd) == -1)
        {
            // /FIXME: ERROR: File streams error.
            error(ERR_IO, FNA);
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

int read_commands()
{
    int cmd_string_size = 0;
    int last_capability;
    char current_char;

    bool is_new_line = true;
    bool parsing_single_quote = false;
    bool parsing_double_quote = false;

    while(is_new_line)
    {
        last_capability = MAX_CMD_STR_LENGTH - cmd_string_size;
        if(last_capability <= 0)
        {
            // /FIXME: ERROR: Commands too long error.
            error(ERR_CMDLG, FNA);
            return -1;
        }
        if(fgets(cmd_string + cmd_string_size, last_capability, stdin) == NULL)
        {
            // /FIXME: ERROR: Commands input error.
            error(ERR_IO, FNA);
            return -1;
        }

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
                else
                {
                    cmd_string[cmd_string_size] = '\0';
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

int parse_commands(const int cmd_string_size)
{
    cmd_infos_num = 0;
    Command_Info* current_cmd_info;

    bool parsing_cmd = false;
    bool parsing_single_quote = false;
    bool parsing_double_quote = false;

    for(int i = 0; i < cmd_string_size; i++)
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

            case '\'':
                if(parsing_double_quote) break;
                parsing_single_quote = !parsing_single_quote;
            
            case '\"':
                if(parsing_single_quote) break;
                parsing_double_quote = !parsing_double_quote;

            default:
                if(!parsing_cmd)
                {
                    if(cmd_infos_num + 1 > MAX_CMD_INFO_NUM)
                    {
                        // /FIXME: ERROR: Commands too many error.
                        error(ERR_CMDMN, FNA);
                        return -1;
                    }
                    parsing_cmd = true;
                    current_cmd_info = &(cmd_infos[cmd_infos_num]);
                    init_cmd_info(current_cmd_info);
                    current_cmd_info->cmd_string_begin = i;
                }
                break;
        }
    }

    return cmd_infos_num;
}

int parse_args(const int cmd_info_index)
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
                        // /FIXME: ERROR: Argument invalid error.
                        error(ERR_ARGIV, FNA);
                        return -1;
                    }
                }

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
                        // /FIXME: ERROR: Argument invalid error.
                        error(ERR_ARGIV, FNA);
                        return -1;
                    }
                }

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

            case '<':
                if(parsing_single_quote || parsing_double_quote) break;

                cmd_string[i] = '\0';
                if(parsing_token)
                {
                    arg_tok_num++;
                    parsing_token = false;
                }

                if(parsing_redir_from_path || parsing_redir_to_path)
                {
                    // /FIXME: ERROR: Redirection argument error.
                    error(ERR_RDRIV, FNA);
                    return -1;
                }
                if(redir_from_num >= MAX_CMD_REDIR_NUM)
                {
                    // /FIXME: ERROR: Redireciton from too many.
                    error(ERR_RDRMN, FNA);
                    return -1;
                }
                parsing_redir_from_path = true;
                current_cmd_info->redirect_from[redir_from_num] = CMD_REDIR_FROM_NRML;
                break;
            
            case '>':
                if(parsing_single_quote || parsing_double_quote) break;

                cmd_string[i] = '\0';
                if(parsing_token)
                {
                    arg_tok_num++;
                    parsing_token = false;
                }

                if(parsing_redir_from_path || parsing_redir_to_path)
                {
                    // /FIXME: ERROR: Redirection argument error.
                    error(ERR_RDRIV, FNA);
                    return -1;
                }
                if(redir_to_num >= MAX_CMD_REDIR_NUM)
                {
                    // /FIXME: ERROR: Redireciton from too many.
                    error(ERR_RDRMN, FNA);
                    return -1;
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
                        redir_from_num++;
                        arg_tok_num--;
                        parsing_redir_from_path = false;
                    }
                    else if(parsing_redir_to_path)
                    {
                        current_cmd_info->redirect_strings[(redir_to_num << 1)+ 1] = &(cmd_string[i]);
                        redir_to_num++;
                        arg_tok_num--;
                        parsing_redir_to_path = false;
                    }
                    else
                    {
                        if(arg_tok_num + 1 >= MAX_ARG_TOK_NUM)
                        {
                            // /FIXME: ERROR: Arguments too many error.
                            error(ERR_ARGMN, FNA);
                            return -1;
                        }
                        current_cmd_info->arg_strings[arg_tok_num] = &(cmd_string[i]);
                    }

                    parsing_token = true;
                }
                break;
        }
    }
    current_cmd_info->arg_strings[arg_tok_num] = NULL;

    return arg_tok_num;
}

int execute_inner(const int cmd_info_index)
{
    Command_Info* cmd_info = &(cmd_infos[cmd_info_index]);
    if(cmd_info->arg_strings[0] != NULL)
    {
        int builtin_counter = -1;
        while(builtin_cmd[++builtin_counter] != NULL)
        {
            if(strcmp(cmd_info->arg_strings[0], builtin_cmd[builtin_counter]) == 0)
            {
                int exit_code = (*builtin_func[builtin_counter])(cmd_info->arg_strings);
                cmd_info->exit_status = exit_code << 8;
                // if(exit_code == EXIT_SUCCESS)
                // {
                //     return EXEC_NORMAL;
                // }
                // else
                // {
                //     // /FIXME: ERROR: Inner command failed error.
                //     error(ERR_CMDIN, FNA);
                //     return EXEC_FAILURE;
                // }
                return EXEC_NORMAL;
            }
        }
    }

    return EXEC_NO_INNER;
}

int execute_outer(const int cmd_info_index)
{
    Command_Info* cmd_info = &(cmd_infos[cmd_info_index]);

    if(cmd_info->arg_strings[0] == NULL)
    {
        // /FIXME: ERROR: Command empty error.
        // error(ERR_CMDEP, FNA);
        return EXEC_EMPTY;
    }

    int file_desc[2];
    if(cmd_info->has_pipe)
    {
        if(pipe(file_desc) == -1)
        {
            // /FIXME: ERROR: Pipe failed error.
            error(ERR_PIPE, FNA);
            return EXEC_FAILURE;
        }
    }

    pid_t exec_pid = fork();
    int stat_loc;
    int exec_status;
    
    if(exec_pid == -1)
    {
        // /FIXME: ERROR: Fork failed error.
        error(ERR_FORK, FNA);
        return EXEC_FAILURE;
    }
    else if(exec_pid == 0)
    {
        if(cmd_info->has_pipe)
        {
            if(close(file_desc[0]) == -1)
            {
                // /FIXME: ERROR: File descriptor operations error.
                error(ERR_IO, FNA);
                exit(EXIT_FAILURE);
            }
            if(set_cmd_io(cmd_info, STDIN_FILENO, file_desc[1]) != SET_IO_SUCCESS)
            {
                // /FIXME: ERROR: Command IO error.
                error(ERR_CMDIO, FNA);
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            if(set_cmd_io(cmd_info, STDIN_FILENO, STDOUT_FILENO) != SET_IO_SUCCESS)
            {
                // /FIXME: ERROR: Command IO error.
                error(ERR_CMDIO, FNA);
                exit(EXIT_FAILURE);
            }
        }

        if(execvp(cmd_info->arg_strings[0], cmd_info->arg_strings) == -1)
        {
            // /FIXME: ERROR: Command executing failed error.
            error(ERR_CMDEX, FNA);
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
                // /FIXME: ERROR: Wait Child failed error.
                error(ERR_WAIT, FNA);
                return EXEC_FAILURE;
            }
            cmd_info->exit_status = stat_loc;
        }
        else
        {
            cmd_info->exit_status = stat_loc = EXIT_BACKGROUND << 8;
        }

        if(cmd_info->has_pipe)
        {
            if(WEXITSTATUS(cmd_info->exit_status) != EXIT_SUCCESS)
            {
                // /FIXME: ERROR: Command failed cannot pipe.
                error(ERR_CMDPP, FNA);
                close(file_desc[1]);
                return EXEC_PIPE_FAILURE;
            }

            if(cmd_info_index + 1 >= cmd_infos_num)
            {
                // /FIXME: ERROR: Pipe to nothing error.
                error(ERR_CMDPP, FNA);
                close(file_desc[1]);
                return EXEC_PIPE_FAILURE;
            }

            cmd_info = &(cmd_infos[cmd_info_index + 1]);
            if(close(file_desc[1]) == -1)
            {
                // /FIXME: ERROR: File streams operations error.
                error(ERR_IO, FNA);
                return EXEC_FAILURE;
            }
            if(set_cmd_io(cmd_info, file_desc[0], STDOUT_FILENO) != SET_IO_SUCCESS)
            {
                // /FIXME: ERROR: Command IO error.
                error(ERR_CMDIO, FNA);
                return EXEC_FAILURE;
            }

            exec_status = EXEC_PIPE;
        }
        else
        {
            exec_status = EXEC_NORMAL;
        }
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

    for(int i = 0; i < MAX_CMD_REDIR_NUM; i++)
    {
        cmd_info->redirect_from[i] = CMD_REDIR_FROM_NONE;
        cmd_info->redirect_to[i] = CMD_REDIR_TO_NONE;

        cmd_info->redirect_strings[i << 1] = NULL;
        cmd_info->redirect_strings[(i << 1) + 1] = NULL;
    }

    cmd_info->is_exec_background = false;
    cmd_info->exec_pid = 0;
    cmd_info->exit_status = EXIT_FAILURE;

    return;
}

int set_cmd_io(const Command_Info* cmd_info, const int input_file_desc, const int output_file_desc)
{
    // PIPE
    if(input_file_desc != STDIN_FILENO)
    {
        if(dup2(input_file_desc, STDIN_FILENO) == -1)
        {
            // /FIXME: ERROR: File streams operations error.
            error(ERR_IO, FNA);
            return SET_IO_FAILURE;
        }
        if(close(input_file_desc) == -1)
        {
            // /FIXME: ERROR: File streams operations error.
            error(ERR_IO, FNA);
            return SET_IO_FAILURE;
        }
    }
    if(output_file_desc != STDOUT_FILENO)
    {
        if(dup2(output_file_desc, STDOUT_FILENO) == -1)
        {
            // /FIXME: ERROR: File streams operations error.
            error(ERR_IO, FNA);
            return SET_IO_FAILURE;
        }
        if(close(output_file_desc) == -1)
        {
            // /FIXME: ERROR: File streams operations error.
            error(ERR_IO, FNA);
            return SET_IO_FAILURE;
        }
    }

    // REDIR
    int redir_from_fd, redir_from_flag, redir_from_fd2;
    for(int i = 0; i < MAX_CMD_REDIR_NUM; i++)
    {
        if(cmd_info->redirect_from[i] == CMD_REDIR_FROM_NONE) break;

        if(cmd_info->redirect_strings[i << 1] == NULL)
        {
            // /FIXME: ERROR: Redirection arguments error.
            error(ERR_RDRIV, FNA);
            return SET_IO_FAILURE;
        }
        switch(cmd_info->redirect_from[i])
        {
            case CMD_REDIR_FROM_NRML:
                redir_from_flag = FILE_OP_FROM;
                redir_from_fd2 = STDIN_FILENO;
                break;
        }
        redir_from_fd = open(cmd_info->redirect_strings[i << 1], redir_from_flag, FILE_PERM_DEFT);
        if(redir_from_fd == NULL)
        {
            // /FIXME: ERROR: File streams operations error.
            error(ERR_IO, FNA);
            return SET_IO_FAILURE;
        }
        if(dup2(redir_from_fd, redir_from_fd2) == -1)
        {
            // /FIXME: ERROR: File streams operations error.
            error(ERR_IO, FNA);
            return SET_IO_FAILURE;
        }
        if(close(redir_from_fd) == -1)
        {
            // /FIXME: ERROR: File streams operations error.
            error(ERR_IO, FNA);
            return SET_IO_FAILURE;
        }
    }
    int redir_to_fd, redir_to_flag, redir_to_fd2;
    for(int i = 0; i < MAX_CMD_REDIR_NUM; i++)
    {
        if(cmd_info->redirect_to[i] == CMD_REDIR_TO_NONE) break;

        if(cmd_info->redirect_strings[(i << 1) + 1] == NULL)
        {
            // /FIXME: ERROR: Redirection arguments error.
            error(ERR_RDRIV, FNA);
            return SET_IO_FAILURE;
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
        redir_to_fd = open(cmd_info->redirect_strings[(i << 1) + 1], redir_to_flag, FILE_PERM_DEFT);
        if(redir_to_fd == NULL)
        {
            // /FIXME: ERROR: File streams operations error.
            error(ERR_IO, FNA);
            return SET_IO_FAILURE;
        }
        if(dup2(redir_to_fd, redir_to_fd2) == -1)
        {
            // /FIXME: ERROR: File streams operations error.
            error(ERR_IO, FNA);
            return SET_IO_FAILURE;
        }
        if(close(redir_to_fd) == -1)
        {
            // /FIXME: ERROR: File streams operations error.
            error(ERR_IO, FNA);
            return SET_IO_FAILURE;
        }
    }

    return SET_IO_SUCCESS;
}

int func_cd(const char** args_string)
{
    struct stat st;

    if(args_string[1] == NULL)
    {
        // /FIXME: ERROR: No argument error.
        error(ERR_CDNAR, FNA);
        return EXIT_FAILURE;
    }

    stat(args_string[1], &st);
    if(!S_ISDIR(st.st_mode))
    {
        // /FIXME: ERROR: Directory not found error.
        error(ERR_CDNDR, FNA);
        return EXIT_FAILURE;
    }

    if(chdir(args_string[1]) != 0)
    {
        // /FIXME: ERROR: Change directory failed error.
        error(ERR_CDFAL, FNA);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int func_help(const char** args_string)
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

int func_exit(const char** args_string)
{
    while(wait(NULL) != -1);
    exit(EXIT_SUCCESS);

    return EXIT_SUCCESS;
}

int error(const int error_type, const char* position)
{
    // WARNING: I/O Errors in this function would be omitted.
    // WARNING: In this function, error messages would be output to the original `stderr`.

    int current_stderr_fd = dup(STDERR_FILENO);
    dup2(stderr_fd, STDERR_FILENO);

    switch(error_type)
    {
        case ERR_NONE:
            fprintf(stderr, is_term_sgr ? TERM_ERR_SGR : TERM_ERR,
                "None Error",
                "Nothing wrong but the error handler called.",
                position); break;

        case ERR_CMPLX:
            fprintf(stderr, is_term_sgr ? TERM_ERR_SGR : TERM_ERR,
                "Complex Error",
                "The error with some complex situations. Seeing the context of errors may help.",
                position); break;

        case ERR_IO:
            fprintf(stderr, is_term_sgr ? TERM_ERR_SGR : TERM_ERR,
                "I/O Error",
                "Something wrong within an I/O operation.",
                position); break;

        case ERR_PIPE:
            fprintf(stderr, is_term_sgr ? TERM_ERR_SGR : TERM_ERR,
                "Pipe Error",
                "Failed to create a pipe to execute commands with pipes.",
                position); break;

        case ERR_FORK:
            fprintf(stderr, is_term_sgr ? TERM_ERR_SGR : TERM_ERR,
                "Fork Error",
                "Failed to fork a process to execute an outer command.",
                position); break;

        case ERR_WAIT:
            fprintf(stderr, is_term_sgr ? TERM_ERR_SGR : TERM_ERR,
                "Wait Error",
                "Failed to wait the child process executing an outer command.",
                position); break;

        case ERR_RDCMD:
            fprintf(stderr, is_term_sgr ? TERM_ERR_SGR : TERM_ERR,
                "Read Commands Error",
                "Failed to read the input string of commands.",
                position); break;

        case ERR_PRCMD:
            fprintf(stderr, is_term_sgr ? TERM_ERR_SGR : TERM_ERR,
                "Parse Commands Error",
                "Failed to parse commands in the input string of commands.",
                position); break;

        case ERR_PRARG:
            fprintf(stderr, is_term_sgr ? TERM_ERR_SGR : TERM_ERR,
                "Parse Arguments Error",
                "Failed to parse arguments in a string of a command.",
                position); break;

        case ERR_EXCMD:
            fprintf(stderr, is_term_sgr ? TERM_ERR_SGR : TERM_ERR,
                "Execute Command Error",
                "Failed to execute the command.",
                position); break;

        case ERR_CMDLG:
            fprintf(stderr, is_term_sgr ? TERM_ERR_SGR : TERM_ERR,
                "Commands Too Long Error",
                "The input string of commands is too long.",
                position); break;

        case ERR_CMDMN:
            fprintf(stderr, is_term_sgr ? TERM_ERR_SGR : TERM_ERR,
                "Commands Too Many Error",
                "There are too many commands in the input string.",
                position); break;

        case ERR_ARGIV:
            fprintf(stderr, is_term_sgr ? TERM_ERR_SGR : TERM_ERR,
                "Argument Invalid Error",
                "There are some problems in parsing an argument. "
                "There may be some arguments not in correct formats.",
                position); break;

        case ERR_ARGMN:
            fprintf(stderr, is_term_sgr ? TERM_ERR_SGR : TERM_ERR,
                "Arguments Too Many Error",
                "There are too many arguments in a command.",
                position); break;

        case ERR_RDRIV:
            fprintf(stderr, is_term_sgr ? TERM_ERR_SGR : TERM_ERR,
                "Redirection Argument Invalid Error",
                "There are some problems in parsing an argument of a redirection expression. "
                "There may be arguments empty or not in correct formats.",
                position); break;

        case ERR_RDRMN:
            fprintf(stderr, is_term_sgr ? TERM_ERR_SGR : TERM_ERR,
                "Redirection Too Many Error",
                "There are too many redirections in a command.",
                position); break;

        case ERR_CMDIN:
            fprintf(stderr, is_term_sgr ? TERM_ERR_SGR : TERM_ERR,
                "Command Inner Error",
                "Failed to execute the inner command.",
                position); break;

        case ERR_CMDIO:
            fprintf(stderr, is_term_sgr ? TERM_ERR_SGR : TERM_ERR,
                "Command I/O Error",
                "Failed to set and do I/O operations of the command within pipe or redirection expressions.",
                position); break;

        case ERR_CMDEP:
            fprintf(stderr, is_term_sgr ? TERM_ERR_SGR : TERM_ERR,
                "Command Empty Error",
                "It is not possible to execute an empty command.",
                position); break;

        case ERR_CMDEX:
            fprintf(stderr, is_term_sgr ? TERM_ERR_SGR : TERM_ERR,
                "Command Executing Error",
                "Failed to execute the command. "
                "Occasionally, the input command might not be found.",
                position); break;

        case ERR_CMDPP:
            fprintf(stderr, is_term_sgr ? TERM_ERR_SGR : TERM_ERR,
                "Command Pipe Error",
                "The command with a pipe to nothing (no more commands to be piped afterward). "
                "Or the command exited not successful so that the pipes cannot continued.",
                position); break;

        case ERR_CDNAR:
            fprintf(stderr, is_term_sgr ? TERM_ERR_SGR : TERM_ERR,
                "CD: No Argument Error",
                "There is no argument to indicate where the directory be changed to.",
                position); break;

        case ERR_CDNDR:
            fprintf(stderr, is_term_sgr ? TERM_ERR_SGR : TERM_ERR,
                "CD: Not Directory Error",
                "The target path does not indicate to a directory.",
                position); break;

        case ERR_CDFAL:
            fprintf(stderr, is_term_sgr ? TERM_ERR_SGR : TERM_ERR,
                "CD: Failure Error",
                "Failed to execute the CD inner command.",
                position); break;

        case ERR_UNK:
        default:
            fprintf(stderr, is_term_sgr ? TERM_ERR_SGR : TERM_ERR,
                "Unknown Error",
                "The unknown error with an unknown error code.",
                position); break;
    }

    fflush(stderr);
    dup2(current_stderr_fd, STDERR_FILENO);
    close(current_stderr_fd);

    return 0;
}
