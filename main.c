#if !defined(_XOPEN_SOURCE)
#define _XOPEN_SOURCE 700
#else
#define _XOPEN_SOURCE 600
#endif

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <time.h>


int fileno(FILE *stream);
// function to convert integer to character
char* itoa(int val, int base);

// function to get lastModification time of the file in given path
static time_t getFileModifiedTime(const char *path);

// function to get size of the file in given path
static int getFileSize(const char *path);

// function to compare position of the files in given paths
static int checkTarPos(const char *path,const char *path2);

// funtion to check for given path is a directory or a file
int is_dir(const char *path);

//function to remove directory after the program finished
int remove_directory(const char *path);

//Paths structure to keep path informations for files and directories
struct Paths {
   char  minPath[1024];                             //Min path holds the paths of unzipped files after extracted ->relative path
   char  fullpath[1024];                            //Full path holds the paths of full path ->absolute path
   time_t  modTime;                                 //Holds last modification time of the file
};

int main (int argc,char **argv)
{
    
    char template[] = "/tmp/tmpdir.XXXXXX";           // Create the temporary directory
    char *tmp_dirname = mkdtemp (template);           // to hold selected files and directories
    char cwd[256];                                    //cwd to keep current working directory path
    char buffer[1024];                                 //Buffer to execute commands
    FILE* file;                                       //file to get selected files from tmp.txt
    struct Paths pathStr[1024];                       //Path struct to hold selected files comes from tmp.txt


    if (tmp_dirname == NULL){                         //Checking for temporary file can be created or failed
    
        perror ("tempdir: error: Could not create tmp directory");
        exit (EXIT_FAILURE);
    }
    
    int i=1;
    
    if (argc<2) {
        perror ("Usage: ./mergetars input_tarfile1 [input_tarfile2 ...] output_tarfile");
        exit (EXIT_FAILURE);
    }
    
    
    while (i<argc-1) {                                          //traversing for command line arguments
            
        char indexedName[1024]="archieve";
            
        strcat(indexedName,itoa(i,10));                         //creates a directory to keep contents of the given archieves
        mkdir(indexedName, 0777);                               //it creates "archieve + index" named directories for archieves
        
        if (fork()==0) {
                                                                
            snprintf(buffer, sizeof(buffer), "archieve%d",i);
            if(execl("/usr/bin/tar","tar","-xvf",argv[i],"-C",buffer,NULL)==-1){   //unzip the archieves to created directories
                perror("ERROR2");
            }
            exit(0);
        }
        else{
            wait(NULL);                                         //Main Process waits Child process
        }
        
        if (fork()==0) {                                        //Execute another file, fork is needed to use exec family
                                                                
            snprintf(buffer, sizeof(buffer), "archieve%d",i);   //sends arguments to buffer
            if(execl("filelist","filelist",buffer,NULL)==-1){   //execute the findPath program to get paths of archieves
                perror("ERROR");
            }
            exit(0);
        }
        else{
            wait(NULL);                                         //Main Process waits Child process
        }
        i++;

    }
    
    
    file = fopen("tmp.txt", "r");                     //Opening file to get selected paths from tmp.txt
    char line[1024];
    
    int index=0;
    
    char* xx = (char*) malloc(0);                     //Created xx dynamically to hold paths from tmp.txt line by line

    while (fgets(line, sizeof(line), file)) {         //Paths from tmp.txt is sent to line
       
        int var=0;                                    //var holds the info if coming path is exist or not in pathsStr structure
        
        char *deneme;                                 //deneme gets the relative path with string operations
        
        int y=0,size=0;
        
        
        for (y=0; line[y]!='\n'; y++) {
            size++;
            xx = (char *) realloc(xx, (size));        //Resizing xx to get path from line dynamically
            xx[y]=line[y];
        }
        xx[y]='\0';                                   //it takes character from line until reaching "\0" character
        
        deneme=strstr(line,"archieve");
        deneme=strtok(deneme, "/");
        deneme = strtok(NULL, "");
        deneme[strlen(deneme)-1]='\0';                //deneme gets the relative path with string operations
                        
        for (int k=0; k<index; k++) {                 //traversing inside of the pathStr structure
            
            if (!strcmp(deneme,pathStr[k].minPath)) {
                                                                            
                time_t t1=getFileModifiedTime(xx);                          //if the upcoming relative path equals to one of path defined before
                time_t t2=pathStr[k].modTime;                               //
                                                                            //checks for the modification times and if new path is updated version
                if (t1>t2) { 
                    printf("modifyTime%s\n",xx);                                           //
                    strcpy( pathStr[k].fullpath, xx);                       //convert the contents of the structure
                    strcpy( pathStr[k].minPath, deneme);
                    pathStr[k].modTime=getFileModifiedTime(xx);
                }
                var=1;                                                      //if the upcoming relative path equals to
                                                                            //one of path defined before it doesnt add again
            }

        }
        
        if (var==0) {
                                                                        
            strcpy( pathStr[index].fullpath, xx);                           //if the upcoming relative path not equals to
            strcpy( pathStr[index].minPath, deneme);                        //one of the path defined before it s added in pathStr structure
            pathStr[index].modTime=getFileModifiedTime(xx);                 //and increases the index of the structure
            index++;
            
        }
    }

    fclose(file);                                                           //closing file
    free(xx);                                                               //free dynamically allocated array
    
    getcwd(cwd,sizeof(cwd));                                                //Keeping the current directory to turn back again later
    
    
    if (chdir (tmp_dirname) == -1)
    {
        perror ("tempdir: error: ");                                        //Changing currrent directory to temporary directory
        exit (EXIT_FAILURE);
    }
    mkdir("final", 0777);
    chdir("final");
    for (int t=index-1; t>=0; t--) {
      printf("forStatement %s\n",pathStr[t].fullpath);
        if (is_dir(pathStr[t].fullpath)) { 
            printf("is_dir %s\n",pathStr[t].fullpath);                             //Traversing inside final selected versions of
            mkdir(pathStr[t].minPath,0777);                                 //the files and move them into temporary directory
        }
        else{
          printf("beforeFork %s\n",pathStr[t].fullpath); 
            if(fork()==0){                                                  //Main Process forks to run exec command with child process
                
                execl("/bin/mv","-i", pathStr[t].fullpath, pathStr[t].minPath, (char *)0);
                printf("fork %s\n",pathStr[t].fullpath);                                                            //Child Process executes the "MOVE" command to move files into temp directory
                exit(0);
            }
            else{
                wait(NULL);                                                 //Main Process waits until child returns
            }
        }
    }
    if (fork()==0) {
                                                            
        snprintf(buffer, sizeof(buffer), "%s/%s", cwd,argv[argc-1]);        //Define the tar utility command to execute
        if(execl("/usr/bin/tar","tar","-cvf",buffer,"final",NULL)==-1){     //Zip final directories and files in temp directory and sends compressed//version to current directory
            perror("ERROR2");
        }
        exit(0);
    }
    else{
        wait(NULL);                                                         //Main Process waits Child process
    }
    
    chdir(cwd);
    
    if (remove_directory(tmp_dirname) == -1){
        perror("tempdir: error: ");                                         // Delete the temporary directory before exit
        exit(EXIT_FAILURE);
    }
    
    remove("tmp.txt");
    
    for (int k=0; k<argc; k++) {
        snprintf(buffer, sizeof(buffer), "archieve%d",k);
        remove_directory(buffer);
    }
    
    return EXIT_SUCCESS;                                                    //Program is Done!
}

// function to convert integer to character on base10
char* itoa(int val, int base){
    
    static char buf[32] = {0};
    
    int i = 30;
    
    for(; val && i ; --i, val /= base)
    
        buf[i] = "0123456789abcdef"[val % base];
    
    return &buf[i+1];
    
}

// function to get lastModification time of the file in given path
static time_t getFileModifiedTime(const char *path)
{
    struct stat attrib;
    stat(path, &attrib);
    return attrib.st_mtime;
}

// funtion to check for given path is a directory or a file
int is_dir(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISDIR(path_stat.st_mode);
}

//function to remove directory after the program finished
int remove_directory(const char *path) {
   DIR *d = opendir(path);
   size_t path_len = strlen(path);
   int r = -1;

   if (d) {
      struct dirent *p;                     //to reach directory contents

      r = 0;
      while (!r && (p=readdir(d))) {        //it gets the content of the given path directory
          int r2 = -1;
          char *buf;
          size_t len;

                                            // Skip the names "." and ".." as we don't want to recurse on them.
          if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
             continue;

          len = path_len + strlen(p->d_name) + 2;
          buf = malloc(len);

          if (buf) {                        //it cleans the content and remove directories recursively
             struct stat statbuf;

             snprintf(buf, len, "%s/%s", path, p->d_name);
             if (!stat(buf, &statbuf)) {
                if (S_ISDIR(statbuf.st_mode))
                   r2 = remove_directory(buf);
                else
                   r2 = unlink(buf);
             }
             free(buf);
          }
          r = r2;
      }
      closedir(d);                          //Close the directory
   }

   if (!r)
      r = rmdir(path);                      //Removes empty directory after cleaning the content

   return r;
}
