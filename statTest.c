#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h> 
#include <sys/stat.h>

typedef enum {
    false = 0, 
    true = 1
} bool; 

int numFilesInDir(char *path); 
char ** newStringArray(int length); 
void deallocateStringArray(char **stringArray, int length); 
char * stringCopy(char *source); 
char ** allFilesInDir(char *path);

int main(int argc, char *argv[]) {
    int numFiles = numFilesInDir("./"); 
    char **files = newStringArray(numFiles); 
    printf("Number of files in directory: %d\n", numFiles); 
    
    deallocateStringArray(files, numFiles); 

   /* 
    struct stat statBuff;

    if (stat("./", &statBuff) == -1) { 
        perror("Failed to get directory status"); 
    }
    else {
        if (S_ISDIR(statBuff.st_mode)) { 
            printf("%d\n", (long long) statBuff.st_size);
        }
        else 
            perror("Not a directory"); 
    }
    */
    return 0;
} 

int numFilesInDir(char *path) { 
    DIR *dirStream;
    int numFiles; 
    dirStream = opendir(path);
    if (dirStream != NULL)  
        for (;readdir(dirStream); numFiles++);
    else 
        perror("Couldn't open the directory");
    (void) closedir(dirStream); 
    return numFiles; 
}


char ** newStringArray(int length) { 
    char ** strArr = calloc(length, sizeof(char*)); 
    return strArr; 
} 

void deallocateStringArray(char **stringArray, int length) { 
    for (int i = 0; i < length; free(stringArray[i++])); 
    free(stringArray); 
} 


char * stringCopy(char *source) { 
    char *strCopy = calloc(strlen(source) + 1, sizeor(char));
    strcpy(strCopy, source); 
    return strCopy; 
} 

/* 
char ** allFilesInDir(char *path) { 
    DIR *dirStream;
    struct dirent *entry; 
    int numFiles = numFilesInDir(path); 
    char **files = newStringArray(numFiles) */
