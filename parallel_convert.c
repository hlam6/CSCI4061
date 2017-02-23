/* Notes ***********************************************************************
 * 
 * TO-DO
 *  * Write contents of junk_buf to file in the parent thread after all of the
 *    children have completed. 
 *  * Write a function creates the output filename from the input filename
 *      * Delete extension from filename
 *      * Append "_<process pid>.jpg" to end of filename
 *  * Set-up logging
 *      * Provided example: image1.png converted to jpg of size 200x200 by process with id : 123456
 *      * Must log to stdout and to a log file 
 *  * Complete convert image function
 *      * fork and then run execv(convert...) in child process
 *  * Write list of junk files to file after all children finished
 *  * Generate HTML page 
*///****************************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h> 
#include <string.h>
#include <errno.h> 
#include <stdbool.h> 
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h> 
#include <sys/stat.h> 
#include <sys/shm.h>
#include <sys/ipc.h> 
#include <dirent.h> 


//_ Constant Definitions _______________________________________________________
#define BUF_SIZE 64
#define MAX_FILES 100 
char *image_ext[4] = {".gif", ".png", ".bmp", NULL}; 


//_ Type Definitions ___________________________________________________________
typedef enum {InvalidArguments, 
              ForkFailed, 
              InvalidConvertCount,
              InvalidDirectory, 
              InvalidOutputDirectory,
              SharedMemFailed,
              NameTooLong, 
              CannotDeleteFile
             } ProgramError; 

typedef enum {GIF = 0, PNG = 1, BMP = 2, NONE} ImageType; 
typedef struct { 
    char files[MAX_FILES][BUF_SIZE]; 
    int num_files; 
    char junk_buf[MAX_FILES][BUF_SIZE];
    int len_jbuf; 
} SharedMemory; 

//_ Global Variables ___________________________________________________________
char output_dir[BUF_SIZE]; 
char input_dir[BUF_SIZE];
int numProcesses; 
ImageType file_type; 
SharedMemory *shmem;
DIR *input_dstream;

//_ Function Prototypes ________________________________________________________
void processArgs(int argc, char *argv[]); 
void error_handler(ProgramError error_type, ...);
ImageType determineImageType(pid_t pid); 
bool check_directory(char *path); 
void check_output_dir(); 
void check_input_dir(); 
void initialize_shared_mem(); 
bool matchesFiletype(char *filename, ImageType type); 
bool ignoreFile(char *name); 
void initialize_input_dir(); 
char * next_file(); 
void processJunkFiles(); 
void processImages(); 
bool isJunk(char *name); 
bool searchProcessedFiles(char *file); 
void check_length(char *name, int max_length);
void delete_junk_file(char *filename); 
void convert_image(char *filename);
void create_pathname(char *pathname, char *filename, char *directory); 
char * get_extension(char *filename); 


//_ Main Function ______________________________________________________________
int main(int argc, char *argv[]) {
    pid_t childpid;

    processArgs(argc, argv);  
    initialize_shared_mem(); 

    for (int i = 0; i < numProcesses && (childpid = fork()); i++); 
    
    if (childpid == -1) {
        error_handler(ForkFailed); 
    }
    else if (childpid == 0) {
        determineFileType(); 
        initialize_input_dir(); 

        // Test output
        printf("I'm a child who converts %s images\n", image_ext[file_type]);  

        if (file_type == NONE) 
            processJunkFiles(); 
        else 
            processImages(); 
    }
    else {
        printf("I am a parent %ld\n", (long) getpid()); // Test output 
        while(wait(NULL) + 1);          // Wait for children to complete
        printf("I waited for my children\n");           // Test output

        // Test output 
        printf("Files found: "); 
        for (int i = 0; i < shmem->num_files; i++) { 
            printf("    %s\n", shmem->files[i]); 
        }
        printf("Junk files: "); 
        for (int i = 0; i < shmem->len_jbuf; i++) { 
            printf("    %s\n", shmem->junk_buf[i]); 
        }

        shmdt(shmem); 
    }
    //fflush(NULL); 
    
    return 0; 
} 
       

//_ Function Definitions _______________________________________________________
void processArgs(int argc, char *argv[]) {
    if (argc != 4) 
        error_handler(InvalidArguments); 
    if (!(numProcesses = atoi(argv[1])) || numProcesses > 10) 
        error_handler(InvalidConvertCount); 
    check_length(argv[2], BUF_SIZE); 
    check_length(argv[3], BUF_SIZE); 
    strcpy(output_dir, argv[2]); 
    strcpy(input_dir, argv[3]); 
    check_output_dir();
    check_input_dir(); 
} 

// Need to modify to kill all child processes when one of them encounters an
// error. Could do using kill, but might need an exit routing to detach shared
// memory and such. 
void error_handler(ProgramError error_type, ...) {
    va_list vargs;
    va_start(vargs, error_type); 
    char err_msg[BUF_SIZE]; 
    char *directory;
    switch (error_type) { 
        case InvalidArguments:    
            errno = EINVAL;
            strcpy(err_msg,  
                  "Usage: parallel_convert convert_count output_dir input_dir"); 
            break; 
        case InvalidConvertCount:
            errno = EINVAL;
            strcpy(err_msg, "convert_count must be between 1 and 10"); 
            break; 
        case ForkFailed: 
            strcpy(err_msg, "fork() failed");
            break;  
        case InvalidDirectory:
            sprintf(err_msg, "Directory \"%s\" is invalid", 
                    va_arg(vargs, char *)); 
            break; 
        case SharedMemFailed: 
            sprintf(err_msg, "Failed to %s shared memory", 
                    va_arg(vargs, char *));
            break; 
        case NameTooLong: 
            sprintf(err_msg, 
                    "\"%s\" exceeds maximum length of %d characters", 
                     va_arg(vargs, char *), BUF_SIZE);
            break; 
        case CannotDeleteFile: 
            sprintf(err_msg, "Cannot delete \"%s\"", va_arg(vargs, char *));
            break;
        default: break; 
    } 
    va_end(vargs); 
    perror(err_msg); 
    exit(1); 
}  


ImageType determineImageType(pid_t pid) {
    int div2 =!((long) pid % 2);
    int div3 =!((long) pid % 3); 
    return div2 && div3 ? GIF : div2 ? PNG : div3 ? BMP : NONE;
} 

bool check_directory(char *path) { 
    struct stat stBuf;
    if (stat(path, &stBuf) != -1)  { 
        if (!S_ISDIR(stBuf.st_mode)) {
            errno = ENOTDIR;
            error_handler(InvalidDirectory, path); 
        } 
        else if (access(path, R_OK | W_OK) == -1)
            error_handler(InvalidDirectory, path);
        else 
            return true; 
    }  
    else 
        return false; 
} 

void check_output_dir() { 
    if (!check_directory(output_dir)) 
        if (mkdir(output_dir, S_IRWXU) == -1) 
            error_handler(InvalidDirectory, output_dir); 
} 


void check_input_dir() {
    if (!check_directory(input_dir)) {
        errno = ENOENT; 
        error_handler(InvalidDirectory, input_dir); 
    } 
} 

// Don't forget to detach shared memory at end of parent process with
// shmdt(shmem). 
void initialize_shared_mem() { 
    key_t key = IPC_PRIVATE; 
    int shmid; 
    if ((shmid = shmget(key, sizeof(SharedMemory), IPC_CREAT | 0666)) == -1) //Creates shared memory of sizeof(SharedMemory ) which is a struct containing the number of files, a char array, and a junk array//
        error_handler(SharedMemFailed, "create");
    if ((shmem = shmat(shmid, NULL, 0)) == (SharedMemory *) -1) 
        error_handler(SharedMemFailed, "attach");
    shmem->num_files = 0; 
    shmem->len_jbuf = 0; 
} 


bool ignoreFile(char *name) { 
    return name[0] == '.';
} 

bool matchesFiletype(char *filename, ImageType type) { 
    int f_len = strlen(filename); 
    int e_len = strlen(image_ext[type]); 
    if (f_len >= e_len) 
        return !strncmp(image_ext[type], &filename[f_len - e_len], e_len);
    return false; 
}

void processJunkFiles() { 
    char *filename; 
    while (filename = next_file()) {
        if (isJunk(filename) && !searchProcessedFiles(filename)) { 
            strcpy(shmem->files[shmem->num_files++], filename);
            strcpy(shmem->junk_buf[shmem->len_jbuf++], filename);
            delete_junk_file(filename); 
        } 
    } 
} 

void processImages() { 
    char *filename; 
    while (filename = next_file()) { 
        if (!ignoreFile(filename) && matchesFiletype(filename, file_type) 
                                  && !searchProcessedFiles(filename)) { 
            strcpy(shmem->files[shmem->num_files++], filename);
            convert_image(filename); 
        } 
    } 
} 
void newFileName(char *filename){ //with strcat, is it possible that we have overlap? Check dest buffer
  char newFile[strlen(filename)-4]; //sizeof str with the extension removed ie; fish.png -> fish
  strncat(newFile, filename, strlen(filename)-4);
  //char *ext = ".jpg";
  strcat(newFile, ".jpg"); 
}

void convert_image(char *filename) { 
  char input_path[BUF_SIZE*2]; 
  char output_path[BUF_SIZE*2];
  char output_path = newFileName(filename);
  // fork 
  // if child, then execv(convert ...)
  // if parent, then wait(NULL) and return 

} 

void create_pathname(char *pathname, char *filename, char *directory) { 
    sprintf(pathname, "%s/%s", directory, filename);  
}  


void initialize_input_dir() { 
    if ((input_dstream = opendir(input_dir)) == NULL) 
        error_handler(InvalidDirectory, input_dir); 
} 

char * next_file() { 
    static struct dirent *entry; 
    if (entry = readdir(input_dstream)) {
        check_length(entry->d_name, BUF_SIZE); 
        return entry->d_name;
    } 
    else 
        return NULL; 
} 


bool isJunk(char *name) { 
    return !(ignoreFile(name) 
             || matchesFiletype(name, GIF)
             || matchesFiletype(name, BMP)
             || matchesFiletype(name, PNG));
} 

bool searchProcessedFiles(char *file) { 
    for (int i = 0; i < shmem->num_files; i++) { 
        if (!strcmp(file, shmem->files[i])) 
            return true; 
    }
    return false; 
} 

void check_length(char *name, int max_length) { 
    if (strlen(name) > max_length) {  
        errno = ENAMETOOLONG; 
        error_handler(NameTooLong, name); 
    } 
}  

// 
void delete_junk_file(char *filename) { 
    char pathname[BUF_SIZE*2]; 
    create_pathname(pathname, filename, input_dir); 
    if (unlink(pathname) == -1) 
        error_handler(CannotDeleteFile, filename); 
} 

char * get_extension(char *filename) { 
    // strtok
} 

