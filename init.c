#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>

int main()
{
    int num;
    FILE* fptr;

    char crtDir[PATH_MAX]; //directorul curent
    char cmd[PATH_MAX] = "PATH=$PATH:"; //comanda de adaugat in bshcr

    if (getcwd(crtDir, sizeof(crtDir)) == NULL) 
    {
        printf("Error getting current directory");
        exit(1);
    }

    fptr = fopen("~/.bashrc", "a"); // deschidere bashrc in modul append
    if (fptr == NULL)
    {
        printf("Error!");
        exit(1);
    }

    strncat(cmd, crtDir, (sizeof(cmd) - strlen(cmd))); //compute command

    fwrite(cmd, 1, sizeof(cmd), fptr);//scriere comanda in fisier

    fclose(fptr);


    return 0;
}
