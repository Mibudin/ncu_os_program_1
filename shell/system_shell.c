#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>


int main()
{
    char* line;
    size_t size;
    bool exit = false;
    bool sgr = (getenv("TERM") != NULL);
    while(!exit)
    {
        line = NULL;
        size = 0;

        fflush(NULL);

        if(sgr)
        {
            printf("\n\001\033[1m\002\001\033[49;34m\002> \001\033[0m\002");
        }
        else
        {
            printf("\n> ");
        }
        getline(&line, &size, stdin);

        if(strcmp(line, "exit\n") == 0)
        {
            exit = true;
        }
        else
        {
            system(line);
        }

        free(line);
    }
    return 0;
}
