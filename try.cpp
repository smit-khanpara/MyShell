#include<stdio.h> 
#include<stdlib.h> 
#include<unistd.h> 
int main() 
{ 
        char *args[]={"ls","-l",NULL}; 
        execvp(args[0],args); 

        printf("Ending-----"); 
      
    return 0; 
} /home/smit/Study/OS/Assignment_1/Code
