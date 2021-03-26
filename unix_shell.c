/*
Phillip Pham
Project 2 - Unix Shell
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int find_pipe(char** cargs)
{
    int indx = 0;
    while(cargs[indx] != '\0')
    {
        if(!strncmp(cargs[indx], "|", 1))
        {
            return indx;
        }
        indx++;
    }
    return -1;
}

int find_pipe(char** carg);

char** parse(char* input, char* carg[])
{
    const char delims[2] = {' ', '\t'};
    char* token;
    int num_args;

    token = strtok(input, delims);
    carg[0] = token;

    char** redirect = malloc(2 * sizeof(char*));

    for (int x = 0; x < 2; x++)
    {
        redirect[x] = malloc(BUFSIZ * sizeof(char));
    }
    
    redirect[0] = "";
    redirect[1] = "";

    while (token != NULL)
    {
        token = strtok(NULL, delims);

        if (token == NULL) break;

        if (!strncmp(token, ">", 1))
        {
            token = strtok(NULL, delims);
            redirect[0] = "o";
            redirect[1] = token;
            return redirect;
        } 

        else if (!strncmp(token, "<", 1))
        {
            token = strtok(NULL, delims);
            redirect[0] = "i";
            redirect[1] = token;
            return redirect;
        }

        else if (!strncmp(token, "|", 1))
        {
            redirect[0] = "p";        
        }
        carg[num_args++] = token;
    }

    return redirect;

}

int main(int argc, const char* argv[])
{
    char input [BUFSIZ];
    char MRU   [BUFSIZ];
    int pipefd[2];

    memset(input, 0, BUFSIZ * sizeof(char));
    memset(MRU,   0, BUFSIZ * sizeof(char));

    while (true)
    {
        printf("osh> ");
        fflush(stdout);
        fgets(input, BUFSIZ, stdin);

        input[strlen(input) - 1] = '\0';

        if (strncmp(input, "exit", 4) == 0)
        {
            return 0;
        }

        if (strncmp(input, "!!", 2))
        {
            strcpy(MRU, input);      
        }

        bool waiting = true;
        char* wait_offset = strstr(input, "&");

        if (wait_offset != NULL)
        {
            *wait_offset = ' '; 
            waiting = false;
        }

        pid_t pid = fork();

        if (pid < 0)
        {
            fprintf(stderr, "failed to fork\n");
            return -1;
        }

        if (pid != 0)
        {  
            if (waiting)
            {
                wait(NULL);
            }
        }

        else
        { 
            char* carg[BUFSIZ];
            memset(carg, 0, BUFSIZ * sizeof(char));

            int history = 0;

            if (!strncmp(input, "!!", 2)) history = 1;
            char** redirect = parse( (history ? MRU : input), carg);

            if (history && MRU[0] == '\0')
            {
                printf("no recently used commands\n");
                exit(0);
            } 

            if (!strncmp(redirect[0], "o", 1))
            {
                printf("output saved to ./%s\n", redirect[1]);
            }

            else if (!strncmp(redirect[0], "i", 1))
            {
                printf("reading from file: ./%s\n", redirect[1]);

                memset(input, 0, BUFSIZ * sizeof(char));
                memset(carg, 0, BUFSIZ * sizeof(char));

                input[strlen(input) - 1]  = '\0';
                parse(input, carg);
            }

            else if (!strncmp(redirect[0], "p", 1))
            {
                char* lhs[BUFSIZ], *rhs[BUFSIZ];

                memset(lhs, 0, BUFSIZ*sizeof(char));
                memset(rhs, 0, BUFSIZ*sizeof(char));

                pid_t pidc;

                int pipe_rhs_offset = find_pipe(carg);
                carg[pipe_rhs_offset] = "\0";
                
                int e = pipe(pipefd);

                if(e < 0)
                {
                    fprintf(stderr, "failed to create pipe\n");
                    return 1;
                }

                for(int x = 0; x < BUFSIZ; x++)
                {
                    int indx = x + pipe_rhs_offset + 1;
                    if(carg[indx] == 0) break;
                    rhs[x] = carg[indx];
                }

                for(int x = 0; x < pipe_rhs_offset; x++)
                {
                    lhs[x] = carg[x];
                }

                pidc = fork();

                if(pidc < 0)
                {
                    fprintf(stderr, "failed to fork\n");
                    return 1;
                }

                wait(NULL);
            }
            execvp(carg[0], carg);
            exit(0);
        }
    }

    return 0;

}


/*
===========================EXAMPLE OUTPUT===========================
osc@ubuntu:~/final-src-osc10e/ch3$ gcc unix_shell.c -o test
osc@ubuntu:~/final-src-osc10e/ch3$ ./test
osh> ls
a.out            DateServer.java  fig3-31.c  fig3-33    fig3-34.c  multi-fork    newproc-posix.c  p2     shell_skeleton.c      shm-posix-producer.c  test  unix_pipe.c   user                win32-pipe-parent.c
DateClient.java  fig3-30.c        fig3-32.c  fig3-33.c  fig3-35.c  multi-fork.c  newproc-win32.c  pid.c  shm-posix-consumer.c  simple-shell.c        u2    unix_shell.c  win32-pipe-child.c  z
osh> cal
     March 2021       
Su Mo Tu We Th Fr Sa  
    1  2  3  4  5  6  
 7  8  9 10 11 12 13  
14 15 16 17 18 19 20  
21 22 23 24 25 26 27  
28 29 30 31           
                      
osh> !!
     March 2021       
Su Mo Tu We Th Fr Sa  
    1  2  3  4  5  6  
 7  8  9 10 11 12 13  
14 15 16 17 18 19 20  
21 22 23 24 25 26 27  
28 29 30 31           
                      
osh> exit
osc@ubuntu:~/final-src-osc10e/ch3$ 
===========================EXAMPLE OUTPUT===========================
*/