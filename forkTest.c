#include <stdio.h>
#include <stdlib.h>
//#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h> 

typedef enum {InvalidArguments } ProgramError; 

void error_handler(ProgramError error_type);
void determineImageType(char *extension); 

int main(int argc, char *argv[]) {
    pid_t childpid;
    int numProcesses; 
    char *image_type; 

    if (argc != 2) 
        error_handler(InvalidArguments); 

    numProcesses = atoi(argv[1]);
    for (int i = 0; i < numProcesses && (childpid = fork()); i++); 
    
    if (childpid == -1) {
        perror("fork() failed"); 
        return 1;
    }
    else if (childpid == 0) {
        determineImageType(image_type); 
    }
    else {
        printf("I am a parent %ld\n", (long) getpid());
        while(wait(NULL) + 1);
        printf("I waited for my children\n");  
    }
    fflush(NULL); 
    
    return 0; 
} 
       



void error_handler(ProgramError error_type) {
    if (error_type == InvalidArguments) 
        printf("Usage: ...\n"); 
    exit(1); 
}  


void determineImageType(char *extension) {
    int div2 =!((long) getpid() % 2);
    int div3 =!((long) getpid() % 3); 
    extension = div2 && div3 ? ".gif" : div2 ? ".png" : div3 ?  ".bmp" : NULL;
    printf("I'm a child who converts %s images\n", extension); 
} 
