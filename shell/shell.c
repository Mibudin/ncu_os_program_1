#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include<unistd.h>
#include<sys/wait.h>


#define TERMINAL_PROMPT "$ "

#define MAX_COMMMAND_LENGTH 256

#define EXIT_SUCCESS 0
#define EXIT_ERROR   1


void shell_init();
int  shell_loop();


int main()
{
    shell_init();

    shell_loop();

    return EXIT_SUCCESS;
}

void shell_init()
{

}

void shell_loop()
{
    char cmd[256];
    char args[128];

    while(true)
    {
        // READ
        // PARSE
        // EXCUTE: fork inside
    }

    return;
}
