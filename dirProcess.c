#include <stdio.h> 
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h> 
#include <sys/stat.h> 

typedef struct {
    char *str;
    int len; 
} String; 

typedef struct { 
    String *arr; 
    int len; 
    int buffSize; 
} StringArray;

typedef enum {
    false = 0, 
    true = 1
} bool; 

String * convertString(char *source);
StringArray * newStringArray(int length);
bool isHidden(char *fname); 
bool isJpeg(String fname); 
int numFilesInDir(char *path);  
void freeStringArray(StringArray *strArray); 
String * newString(int length);
bool matchesExtension(char *extension, char *filename); 
StringArray * allFilesInDir(char *path); 
StringArray * findImageFiles(char *path); 
void addString(String string, StringArray *stringArr); 


int main(int argc, char *argv[]) { 
    StringArray files = *allFilesInDir("./"); 
    printf("Number of files in directory: %d\n", files.len);
    for (int i = 0; i < files.len; i++)
        printf("File: %s\n", files.arr[i].str); 
    StringArray imageFiles = *findImageFiles("./"); 
    printf("Number of image files: %d\n", imageFiles.len);
    for (int i = 0; i < imageFiles.len; i++) 
        printf("Image File: %s\n", imageFiles.arr[i].str); 
    freeStringArray(&imageFiles); 
    freeStringArray(&files);
    return 0; 
}


bool isHidden(char *fname) { 
    return fname[1] == '.';
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

void freeStringArray(StringArray *strArray) { 
   for (int i = 0; i < strArray->buffSize; free(strArray->arr[i++].str));
   free(strArray->arr);
} 

String * newString(int length) { 
    String *newStr = malloc(sizeof(String)); 
    newStr->len = length;
    newStr->str = calloc(length, sizeof(char)); 
    return newStr; 
}

// Usage: String newStr = *convertString(sourceStr); 
String * convertString(char *source) { 
    String *newStr = malloc(sizeof(String));
    newStr->len = strlen(source) + 1; 
    newStr->str = calloc(newStr->len, sizeof(char)); 
    strcpy(newStr->str, source); 
    return newStr; 
} 


StringArray * newStringArray(int length) { 
    StringArray *newArr = malloc(sizeof(StringArray)); 
    newArr->len = 0;
    newArr->buffSize = length; 
    newArr->arr = calloc(length, sizeof(String));
    return newArr; 
} 

bool matchesExtension(char *extension, char *filename) {  
    int extLen = strlen(extension); 
    int fnameLen = strlen(filename);
    printf("extLen = %d, fnameLen = %d\n", extLen, fnameLen);  
    if (fnameLen > extLen) { 
        for (int i = 1; i <= extLen; i++) {
            printf("%c == %c?\n", filename[fnameLen - i], extension[extLen - i]);
            if (filename[fnameLen - i] != extension[extLen - i]) 
                return false;
        }
        return true; 
    }
    else 
        return false;
} 

void addString(String string, StringArray *stringArr) { 
    if (stringArr->len < stringArr->buffSize) {
        stringArr->arr[stringArr->len] = string;
        stringArr->len++; 
    }
} 

     

StringArray * allFilesInDir(char *path) { 
    struct dirent *entry; 
    DIR *dirStream = opendir(path); 
    StringArray *files = newStringArray(numFilesInDir(path));
 
    if (dirStream != NULL) { 
        while (entry = readdir(dirStream))
            addString(*convertString(entry->d_name), files);  
        (void) closedir (dirStream); 
    }
    else 
        perror ("Couldn't open the directory"); 
    return files; 
} 

     
StringArray * findImageFiles(char *path) {
    StringArray *allFiles = allFilesInDir(path); 
    StringArray *imageFiles = newStringArray(allFiles->len);
    StringArray *otherFiles = newStringArray(allFiles->len);  
    struct stat stBuf;
    for (int i = 0; i < allFiles->len; ++i) { 
        if (stat(allFiles->arr[i].str, &stBuf) != -1 && S_ISREG(stBuf.st_mode)) {

            printf("A: %d\n", i); 
            if (!isHidden(allFiles->arr[i].str) 
                     && (matchesExtension(".jpg", allFiles->arr[i].str) 
                         || matchesExtension(".png", allFiles->arr[i].str) 
                         || matchesExtension(".bmp", allFiles->arr[i].str))) {
                addString(allFiles->arr[i], imageFiles); 
                printf("Added %s to imageFiles \n", allFiles->arr[i].str); 
            }
            else {
                addString(allFiles->arr[i], otherFiles); 
                printf("Added %s to otherFiles \n", allFiles->arr[i].str); 
            }
        }
    }
    freeStringArray(allFiles);
    freeStringArray(otherFiles); 
    return imageFiles; 
    // Create and open a file for not images 
    


}  
/* Graveyard 
char * allocateAndCopyString(char *source) { 
    char *strCopy = calloc(strlen(source) + 1, sizeof(char));
    strcpy(strCopy, source); 
    return strCopy; 
}
*/ 
