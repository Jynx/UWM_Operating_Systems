#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
 
#define DELIMS " \t\r\n"
#define INPUTSIZE 1025
#define PWDSIZE 256
 
int parseInput(char** cmd, char* tokens, int* redirect, int* pipeFlag, int* append, int* pipemark){
 
    int i = 0, j = 0;
    while((cmd[i] = strtok(tokens, DELIMS)) != NULL) {
        tokens = NULL;
        i++;
        assert(i < 50);  
    }
 
    for (j = 0; j < i; j++) {
        if(strcmp(cmd[j], ">") == 0) {
            *redirect = 1;
            break;
        }
        if(strcmp(cmd[j], "|") == 0) {
            *pipeFlag = 1;
            *pipemark = j;
            break;
        }
        if(strcmp(cmd[j], ">>") == 0) {
            *append = 1;
            break;
        }
    }
    return i;
}
 
int main() {
 
    char* shellName = "mysh> ";
    char line[INPUTSIZE];
 
    while(fputs(shellName, stdout) > 0 && fgets(line, sizeof(line), stdin) != NULL) {
        char* cmd[50];
        char* tokens = line;
        int count = 0;
        int redirect = 0, pipeFlag = 0, append = 0, pipemark = 0;
 
        count = parseInput(cmd, tokens, &redirect, &pipeFlag, &append, &pipemark);
 
       // printf("redirect:%d, pipe:%d, append%d\n", redirect, pipeFlag, append);
 
        
        if(cmd[0] == NULL) {
            fprintf(stdout, "gothere\n");
            fprintf(stderr, "Error!\n"); // blank line
            continue;
        }

        if(strcmp(cmd[0], "exit") == 0) {
            if(count == 1) { // exit with more than 1 arg like :exit blarhdsg
                exit(1);
            }
            else {
                fprintf(stderr, "Error!\n");
                continue;
            }    
        }
 
        else if(strcmp(cmd[0], "cd") == 0) {
            char* arg = cmd[1];
            if (!arg) {
                chdir(getenv("HOME"));
            }
            else if (chdir(arg) == -1) {  // couldn't change to arg directory
                fprintf(stderr, "Error!\n");
                continue;
            }
        }    
 
        else if(strcmp(cmd[0], "pwd") == 0) {
            char pwd[PWDSIZE];
            char* s;
            s = getcwd(pwd, sizeof(pwd));
            if (s == NULL) {
                perror("getcwd error");
            }
            else {
                printf("%s\n", pwd);
            }
        }
 
        else {
            //printf("This is process: (pid:%d\n", (int) getpid());
 
 
            //figure out how to split input
            int pfd[2];
            //printf("pfd1: %d\n", pfd[1]);
            if (pipeFlag == 1 && pipe(pfd) == -1) {
                fprintf(stderr, "pipe failed\n");
                exit(1);
            }    
            int child = fork(); //fork(); can return -1
            if (child < 0) {
                fprintf(stderr, "fork failed\n");
                exit(1);
            }
            else if(child == 0) {
                if(redirect == 1) {
                    char* fileName = cmd[count-1];
                    if(strcmp(fileName, ">") == 0) {
                        fprintf(stderr, "Error!\n");
                        break;
                    }
                    cmd[count - 2] = '\0';
                    int outfile = open(fileName, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
                    dup2(outfile, 1);
                    //file descriptor?
                }
 
                if(append == 1) {
                    char* fileName = cmd[count-1];
                    if(strcmp(fileName, ">>") == 0) {
                        fprintf(stderr, "Error!\n");
                        break;
                    }
                    cmd[count - 2] = '\0';
                    printf("The Filename is: %s\n", fileName);
                    int outfile = open(fileName, O_CREAT | O_WRONLY | O_APPEND, S_IRWXU);
                    dup2(outfile, 1);
                    //close file descriptor?
                }
 
                if(pipeFlag == 1) {
                    close(1);
                    dup(pfd[1]); // Replace standard output with output part of pipe
                    close(pfd[0]); // close input side of pipe
                    close(pfd[1]);
                    cmd[pipemark] = NULL;
                }               
                if ( (execvp(cmd[0],cmd) == -1) ) {
                    fprintf(stderr, "Error!\n");
                    break;
                }    
                printf("should not get here\n");
                return 0;
            }
            else {
                // printf("Hello, I am parent of %d: (wd:%d) (pid:%d)\n", child, wc, (int) getpid());
                if (pipeFlag == 1) {        
                    pipeFlag = 0;
                    child = fork(); // fork(); can return -1
                    if (child < 0) {
                        fprintf(stderr, "fork failed\n");
                        exit(1);
                    }
                    else if(child == 0) {
                        close(0);
                        // Replace standard input with input part of pipe
                        dup(pfd[0]); 
                        close(pfd[1]); // close output side of pipe
                        close(pfd[0]);
                        if( (execvp(cmd[pipemark + 1], &cmd[pipemark + 1])) == -1) {
                            fprintf(stderr, "Error!\n");
                        }    
                        return 0;
                    }
                    else {
                        close(pfd[0]);
                        close(pfd[1]);
                        int wc = wait(0);
                        wc = wait(0);
                        //printf("Hello, I am parent of %d: (wd:%d) (pid:%d)\n", child, wc, (int) getpid());
                    }
                    
                } else {
                    int wc = wait(0);
                }
            }
        }
        // check for return errors, where to and where not to use errno
        // wait not working?
    }
    return 0;
}