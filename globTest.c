#include <glob.h>
#include <stdlib.h> 
#include <stdio.h>
#include <string.h> 

void allocateAndCopyString(char *source, char *destination) { 
    destination = calloc(strlen(source) + 1, sizeof(char)); 
    strcpy(destination, source); 
} 

int getFiles(char *pattern, char *fileBuffer[], int startIndex) {
    glob_t globBuffer; 
    int numFiles; 
    glob(pattern, 0, NULL, &globBuffer); 
    numFiles = globBuffer.gl_pathc; 
    for (int i = 0; i < numFiles; i++) {
       /* 
        fileBuffer[i + startIndex] = calloc(strlen(globBuffer.gl_pathv[i])+1,
                sizeof(char)); 
        strcpy(fileBuffer[i + startIndex], globBuffer.gl_pathv[i]); 
        */ 
        allocateAndCopyString(globBuffer.gl_pathv[i], fileBuffer[i + startIndex]);  
    }

    if (globBuffer.gl_pathc > 0) 
        globfree(&globBuffer); 
    return numFiles; 
} 



char ** allocateDynamicStringArray(int length) { 
    char **array = malloc(length * sizeof(char*)); 
    return array; 
} 

void deallocateStringArray(char **array, int length) { 
    for (int i = 0; i < length; free(array[i++]));
    free(array);
    array = NULL; 
}

int findString(char *searchStr, char *searchArr[], int length) { 
    for (int i = 0; i < length; i++ ) 
        if (searchStr == searchArr[i])  
            return i;
    return -1; 
}

int main(int argc, char *argv[]) {
   /*  
    char *allFiles[128]; 
    char *imageFiles[128];
    char *patterns[3] = {"*.jpg", "*.png", "*.bmp"}; 
    glob_t imageFileBuf;
    glob_t allFileBuf; 
    glob(patterns[1],0, NULL, &imageFileBuf); 
    glob("*", 0, NULL, &allFileBuf); 
   

    char *str1 = "something";
    char *str2 = "something";
    char *str3 = "something else"; 
    printf("str1 = str2 => %d\n", (str1 == str2));  
    */
    char **files = allocateDynamicStringArray(256);   
    int numFiles = 0; 
    char *patterns[3] = {"*.jpg", "*.png", "*.bmp"}; 
    for (int i = 0; i < 3; i++) { 
       numFiles += getFiles(patterns[i], files, numFiles); 
    }  

    for (int i = 0; i < numFiles; i++) { 
        printf("%s\n", files[i]); 
    }
     
    printf("%d\n", sizeof(files));
    deallocateStringArray(files, numFiles); 
    printf("%d\n", sizeof(files));

    return 0;  
} 
