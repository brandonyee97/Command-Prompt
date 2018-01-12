#ifndef SFISH_H
#define SFISH_H
#include <readline/readline.h>
#include <readline/history.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <string.h>
#include <signal.h>
#include <wait.h>
#include <ucontext.h>

char *makepwd(char* prompt, char* cwd);
void executable(int redirect, char* redirects[], char* args[], char* envp[], int superRed, char* superRedirects[]);
void alarmHandler(int sig);
void childHandler(int sig, siginfo_t *sip, void *notused);
void userHandler(int sig);
#endif
