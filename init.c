#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>

int main()
{
   int num;
   FILE *fptr;

   char crtDir[PAHT_MAX]; //current directory
   char cmd[PAHT_MAX] = "PATH=$PATH:"; //command to be added in bshcr

   if(getcwd(crtDir, sizeOf(crtDir)) == NULL) 
   {
 	perror("Error getting current directory");
        return errno;
   }

   fptr = fopen("~/.bashcr","a"); // open bashcr in append mode

   if(fptr == NULL) 
   {
      perror("Error opening bashcr!");   
      return errno;             
   }

   strncat(cmd, crtDir, (sizeof(cmd) - strlen(cmd))); //compute command

   fwrite(cmd, 1, sizeof(cmd), fptr); // write command to file

   fclose(fptr);


   return 0;
}