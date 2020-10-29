#include <stdio.h>
#include <stdlib.h>
#include <dirent.h> 
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <limits.h>
#include <time.h>
#include "filelist.h"

//recursive function to find all contents of given directory
void postOrderApply(char* path);

//checks the given path is file or directory
int is_regular_file(const char *path);

//checks the given path is file or directory
int is_dir(const char *path);

int main(int argc, char** argv) {
    
    
    char cwd[256];                                  //keeps current working directory
    
    getcwd(tmpFile,sizeof(tmpFile));
    strcat(tmpFile,"/tmp.txt");                     //Merge current directory and tmp.txt file paths

    if(argc==2){                                    //Checks for argument size
 
        if(argv[1][0]=='/'){
            strcpy(cwd,argv[1]);
        }                                           //String operations to merge current directory and given argument
        else{
            getcwd(cwd,sizeof(cwd));
            strcat(cwd,"/");
            strcat(cwd,argv[1]);
        }
            
        postOrderApply(cwd);                        //Function call for getting contents of the given path

    }
    else
    {
        printf("Wrong Format! Usage: ./findPath Path \n");
    }
            
    return 0;
    
}

