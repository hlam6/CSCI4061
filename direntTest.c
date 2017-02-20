#include <stdio.h> 
#include <sys/types.h>
#include <dirent.h> 

int main(int argc, char *argv[]) { 
    DIR *dirStream;
    struct dirent *entry; 
    dirStream = opendir("./"); 
    
    if (dirStream != NULL) { 
        while (entry = readdir(dirStream)) {
            puts(entry->d_name); 
        } 
        (void) closedir (dirStream); 
    }
    else {
        perror ("Couldn't open the directory"); 
    }
    return 0; 
}

