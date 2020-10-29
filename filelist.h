
#define XOPEN_SOURCE >= 600 || _XOPEN_SOURCE && _XOPEN_SOURCE_EXTENDED

//Variables

char tmpFile[256];

int fileno(FILE *stream);

//checks the given path is file or directory
int is_regular_file(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

//checks the given path is file or directory
int is_dir(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISDIR(path_stat.st_mode);
}

//recursive function to find all contents of given directory
void postOrderApply(char* path){
    
    struct dirent *de;                        //dirent structure to reach content of directories
    char cwd[256];                            //Current Working Directory
    char old[256];                            //Old Current Working Directory
    FILE* file;                               //File to read tmp.txt content
    DIR *dr;                                  //Directory for read directories
    
    strcpy(cwd,path);
    chdir(cwd);                               //Change directory to given path
    dr = opendir(cwd);                        //opens the directory
    strcpy(old,cwd);                          //keeps old current directories
    
    if (dr == NULL)
    {
        printf("Could not open current directory -> %s",cwd);
    }
  
    else{
        while ((de = readdir(dr)) != NULL){   //Reads content of directory
        
            strcat(cwd,"/");
            strcat(cwd,de->d_name);           //Merge content's path with current directory
            
            if(is_regular_file(cwd)){         //Checks the path if it s file or directory
                
                file = fopen (tmpFile, "a");
                flock(fileno(file),2);
                                              //Writes the path into tmp.txt
                fprintf(file,"%s\n",cwd);
                flock(fileno(file),8);
                fclose(file);
                
            }
            else if(is_dir(cwd)){             //Checks the path if it s file or directory
                if(strcmp(de->d_name,".")!=0 && strcmp(de->d_name,"..")!=0){
                                              //We dont want to control "." and ".." directories
                    
                    if(fork()==0){            //if its a directory path, we traverse inside of the directory recursively
                        
                        postOrderApply(cwd);  //Sends child process to inside of the current directory

                        file = fopen (tmpFile, "a");
                        flock(fileno(file),2);
                                              //Writes the path into tmp.txt
                        fprintf(file,"%s\n",cwd);
                        flock(fileno(file),8);
                        fclose(file);

                        exit(0);

                    }
                    else{
                        wait(NULL);           //Parent Process waits for child process
                    }
                }
            }
            strcpy(cwd,old);                  //When it s finished, it returns the old current directory
        }
    }

    closedir(dr);                             //Close directories
}

