#include "sfish.h"
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

char* makepwd(char* prompt, char* cwd)
{
    strcpy(prompt, "<bryee> : <");
    strcat(prompt, cwd);
    strcat(prompt, "> $ ");
    return prompt;
}

void executable(int redirect, char* redirects[], char* args[], char* envp[], int superRed, char* superRedirects[])
{
          int ifExists;

          //if there is a redirection
          if(redirect == 1)
          {
            int fileDescript;
            if(redirects[0] != NULL)//if there is a <
              {
                fileDescript = open(redirects[1], O_RDONLY);
                if(fileDescript != -1)
                {
                  dup2(fileDescript, STDIN_FILENO);
                  close(fileDescript);
                }
                else
                {
                  fprintf(stderr, "%s\n", "Error in file/file does not exist.");
                  return;
                }
              }
              if(redirects[2] != NULL)//if there is a >
              {
                fileDescript = open(redirects[3], O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
                if(fileDescript != -1)
                {
                  dup2(fileDescript, STDOUT_FILENO);
                  close(fileDescript);
                }
                else
                {
                  fprintf(stderr, "%s\n", "Error in file");
                  return;
                }
              }
            }

          if(superRed == 1)
          {
            int fileDescript;
            if(superRedirects[0] != NULL)//if there is a 1>
              {
                fileDescript = open(superRedirects[1], O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
                if(fileDescript != -1)
                {
                  dup2(fileDescript, STDOUT_FILENO);
                  close(fileDescript);
                }
                else
                {
                  fprintf(stderr, "%s\n", "Error in file/file does not exist.");
                  return;
                }
              }
             else if(superRedirects[2] != NULL)//if there is a 2>
              {
                fileDescript = open(superRedirects[3], O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
                if(fileDescript != -1)
                {
                  dup2(fileDescript, STDERR_FILENO);
                  close(fileDescript);
                }
                else
                {
                  fprintf(stderr, "%s\n", "Error in file");
                  return;
                }
              }
              else if(superRedirects[4] != NULL)//if there is a &>
              {
                fileDescript = open(superRedirects[5], O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
                if(fileDescript != -1)
                {
                  dup2(fileDescript, STDOUT_FILENO);
                  dup2(fileDescript, STDERR_FILENO);
                  close(fileDescript);
                }
                else
                {
                  fprintf(stderr, "%s\n", "Error in file");
                  return;
                }
              }
              else if(superRedirects[6] != NULL)//if there is a >>
              {
                fileDescript = open(superRedirects[7], O_CREAT|O_APPEND|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
                if(fileDescript != -1)
                {
                  dup2(fileDescript, STDOUT_FILENO);
                  close(fileDescript);
                }
                else
                {
                  fprintf(stderr, "%s\n", "Error in file");
                  return;
                }
              }
            }

            if(strcmp(args[0], "pwd") == 0)
            {
              if(args[1] == NULL)
                printf("%s\n", getcwd(NULL, 0));
              else
                printf("Too many args in pwd\n");
            }
            else if(strchr(*args, '/') != NULL)  //IF THERE IS A /
            {
              struct stat buf;
              ifExists = stat(args[0], &buf);
              if(ifExists == 0)//if file exists
              {
                execve(*args, args, envp);
              }
              else
              {
                perror("Error: ");
              }
            }
            else  //NO /, SO SEARCH FOR IT
            {
              char* paths[4096];
              int numPaths = 0;
              char* findPath = strtok(getenv("PATH"), ":");
              while(findPath != NULL)
              {
                paths[numPaths] = findPath;
                findPath = strtok(NULL, ":");
                numPaths++;
              }
              paths[numPaths+1] = NULL;
              int i = 0;
              int exists = 0;
              char fake[4096];
              struct stat buf;
              while(paths[i] != NULL)
              {
                strcpy(fake, paths[i]);
                strcat(fake, "/");
                strcat(fake, args[0]);
                if(stat(fake, &buf) == 0)//exists
                {
                  exists = 1;
                  break;
                }
                i++;
              }
              if(exists == 1)
              {
                execve(fake, args, envp);
              }
              else
              {
                fprintf(stderr, "Error: command not found\n");
              }
            }
}

void alarmHandler(int sig)
{
  char* numStr = getenv("ALARM");
  char str[4096];
  strcpy(str, "\nYour ");
  strcat(str, numStr);
  strcat(str, " second timer has finished!\n");
  write(STDOUT_FILENO, str, strlen(str));

  char prompt[4096];
  strcpy(prompt, "<bryee> : <");
  strcat(prompt, getenv("PWD"));
  strcat(prompt, "> $ ");
  write(STDOUT_FILENO, prompt, strlen(prompt));
}

void childHandler(int sig, siginfo_t *sip, void *notused)
{
  char str[4096];
  strcpy(str, "Child with PID &");
  char numStr[4096];
  int num = sip->si_pid;
  sprintf(numStr, "%d", num);
  strcat(str, numStr);
  strcat(str, " has died. It spent ");
  char secondiStr[4096];
  int secondi = (double)(sip->si_utime + sip->si_stime) / (sysconf(_SC_CLK_TCK)) * 1000;
  sprintf(secondiStr, "%d", secondi);
  strcat(str, secondiStr);//CHANGE
  strcat(str,  " milliseconds utilizing the CPU.\n");
  write(STDOUT_FILENO, str, strlen(str));
  //double numerator = (sip->si_utime + sip->si_stime) / (sysconf(_SC_CLK_TCK));
  //printf("Child with PID &%i has died. It spent %i milliseconds utilizing the CPU.\n", sip->si_pid, secondi);
}

void userHandler(int sig)
{
  char* str = "Well that was easy.\n";
  write(STDOUT_FILENO, str, strlen(str));
}
