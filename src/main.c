#include "sfish.h"
#include "debug.h"
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

/*
 * As in previous hws the main function must be in its own file!
 */

int main(int argc, char const *argv[], char* envp[]){
    /* DO NOT MODIFY THIS. If you do you will get a ZERO. */
    rl_catch_signals = 0;
    /* This is disable readline's default signal handlers, since you are going to install your own.*/
    if(signal(SIGALRM, alarmHandler) == SIG_ERR)
    	fprintf(stderr, "Signal Error\n");

    struct sigaction action;
    action.sa_sigaction = childHandler;
    sigfillset(&action.sa_mask);
    action.sa_flags = SA_SIGINFO;
    sigaction(SIGCHLD, &action, NULL);

    if(signal(SIGUSR2, userHandler) == SIG_ERR)
    	fprintf(stderr, "Signal Error\n");

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGTSTP);
    sigprocmask(SIG_BLOCK, &set, NULL);

    char *cmd;
    char *cwd = getcwd(NULL, 0);
    setenv("PWD", cwd, 1);
    char promptArray[4096];
    char* prompt = makepwd(promptArray, cwd);
    char* prevwd = NULL;
    pid_t pid;
    while((cmd = readline(prompt)) != NULL) {
    	//get args
	    char* getArgs = strtok(cmd, " ");
    	char* args[4096];																		//array of args, arg[0] is exec
       	int numArgs = 0;
    	while(getArgs != NULL && strcmp(getArgs, "<") != 0 && strcmp(getArgs, ">") != 0 && strcmp(getArgs, "|") != 0
    	 && strcmp(getArgs, "1>") != 0 && strcmp(getArgs, "2>") != 0 && strcmp(getArgs, "&>") != 0 && strcmp(getArgs, ">>") != 0)//get args
    	{
        	args[numArgs] = getArgs;
        	getArgs = strtok(NULL, " ");
        	numArgs++;
        }
       	args[numArgs+1] = NULL;

       	//redirects
       	int redirect = 0;
       	int superRed = 0;
       	char* redirects[4];//stores REDIRECTS, 0 is <, 2nd is arg, 3rd is >, last is arg
	    for(int i = 0; i < 4; i++)
	   		redirects[i] = NULL;
	   	char* superRedirect[8];
	   	for(int i = 0; i < 8; i++)
	   		superRedirect[i] = NULL;
	   	int justGotLeft = 0;
	   	int justGotRight = 0;
	   	int justGotOut = 0;
	   	int justGotErr = 0;
	   	int justGotBoth = 0;
	   	int justGotAppend = 0;

	   	//pipes
	   	int pipe1 = 0;
	    char* args1[4096];																		//array of args, arg[0] is exec
	    int numArgs1 = 0;
	    int mypipes[2];

		int pipe2 = 0;
	    char* args2[4096];																		//array of args, arg[0] is exec
	    int numArgs2 = 0;
	    int mypipes1[2];


	    //redirect
    	if(getArgs != NULL && (strcmp(getArgs, "<") == 0 || strcmp(getArgs, ">") == 0))
    	{
    		redirect = 1;
	       	while(getArgs != NULL)
	       	{
	       		if(strcmp(getArgs, "<") == 0 || strcmp(getArgs, ">") == 0)
	    		{
	    			if(strcmp(getArgs, "<") == 0 && justGotLeft == 0)
	    			{
	    				redirects[0] = getArgs;
	    				justGotLeft = 1;
	    			}
	    			else if(strcmp(getArgs, ">") == 0 && justGotRight == 0)
	    			{
	    				redirects[2] = getArgs;
	    				justGotRight = 1;
	    			}
	        	}
	        	else if(justGotLeft == 1)
	        	{
	        		justGotLeft = -1;
	        		redirects[1] = getArgs;
	        	}
	        	else if(justGotRight == 1)
	        	{
	        		justGotRight = -1;
	        		redirects[3] = getArgs;
	        	}
	        	getArgs = strtok(NULL, " ");
	       	}
	    }
	    //super redirect
	    else if(getArgs != NULL && (strcmp(getArgs, "1>") == 0 || strcmp(getArgs, "2>") == 0 || strcmp(getArgs, "&>") == 0 || strcmp(getArgs, ">>") == 0))
	    {
	    	superRed = 1;
	    	while(getArgs != NULL)
	       	{
	       		if(strcmp(getArgs, "1>") == 0 || strcmp(getArgs, "2>") == 0 || strcmp(getArgs, "&>") == 0 || strcmp(getArgs, ">>") == 0)
	    		{
	    			if(strcmp(getArgs, "1>") == 0 && justGotOut == 0)
	    			{
	    				superRedirect[0] = getArgs;
	    				justGotOut = 1;
	    			}
	    			else if(strcmp(getArgs, "2>") == 0 && justGotErr == 0)
	    			{
	    				superRedirect[2] = getArgs;
	    				justGotErr = 1;
	    			}
	    			else if(strcmp(getArgs, "&>") == 0 && justGotBoth == 0)
	    			{
	    				superRedirect[4] = getArgs;
	    				justGotBoth = 1;
	    			}
	    			else if(strcmp(getArgs, ">>") == 0 && justGotAppend == 0)
	    			{
	    				superRedirect[6] = getArgs;
	    				justGotAppend = 1;
	    			}
	        	}
	        	else if(justGotOut == 1)
	        	{
	        		justGotOut = -1;
	        		superRedirect[1] = getArgs;
	        	}
	        	else if(justGotErr == 1)
	        	{
	        		justGotErr = -1;
	        		superRedirect[3] = getArgs;
	        	}
	        	else if(justGotBoth == 1)
	        	{
	        		justGotBoth = -1;
	        		superRedirect[5] = getArgs;
	        	}
	        	else if(justGotAppend == 1)
	        	{
	        		justGotAppend = -1;
	        		superRedirect[7] = getArgs;
	        	}
	        	getArgs = strtok(NULL, " ");
	       	}
	    }

		//pipe
	    else if(getArgs != NULL && strcmp(getArgs, "|") == 0)
	    {
	    	pipe1 = 1;
	    	getArgs = strtok(NULL, " ");
	    	while(getArgs != NULL && strcmp(getArgs, "|") != 0)//get args
	    	{
	        	args1[numArgs1] = getArgs;
	        	getArgs = strtok(NULL, " ");
	        	numArgs1++;
	        }
	       	args1[numArgs1+1] = NULL;

	       	if(getArgs != NULL && strcmp(getArgs, "|") == 0)
	       	{
	       		pipe2 = 1;
		    	getArgs = strtok(NULL, " ");
		    	while(getArgs != NULL && strcmp(getArgs, "|") != 0)//get args
		    	{
		        	args2[numArgs2] = getArgs;
		        	getArgs = strtok(NULL, " ");
		        	numArgs2++;
		        }
		       	args2[numArgs2+1] = NULL;
	       	}
	    }

	    if(args[0] == NULL)
	    	fprintf(stderr, "Error: invalid command.\n");
        else if (strcmp(args[0], "exit")==0)
        {
        	if(args[1] == NULL && redirect == 0 && superRed == 0 && pipe1 == 0)
            	exit(0);
            else
            	fprintf(stderr, "Error: too many arguments\n");
        }
        else if (strcmp(args[0], "help")==0)
        {
        	if(args[1] == NULL && redirect == 0 && superRed == 0 && pipe1 == 0)
        		printf("help: Print a list of all builtins and their basic usage.\n"
                "exit: Exits the shell.\n"
                "cd: Changes the current working directory of the shell.\n"
                "\t cd - should change the working directory to the last directory you were in.\n"
                "\t cd with no arguments should go to your home directory.\n"
                "\t cd . will leave you in the same directory you are currently in (i.e. your current directory won't change).\n"
                "\t cd .. will move you up one directory. So, if you are /usr/bin/tmp, cd .. moves you to /usr/bin.\n"
                "pwd: prints the absolute path of the current working directory.\n"
                );
        	else
        		fprintf(stderr, "Error: too many arguments\n");
        }
        else if (strcmp(args[0], "cd")==0)
        {
        	if(redirect != 0 || superRed != 0 || pipe1 != 0)
        		fprintf(stderr, "%s\n", "Error: invalid arguments");
        	else
        	{
	        	if(args[1] == NULL)
	        	{
		        	char *homeDirec = getenv("HOME");
		        	int changeDir = chdir(homeDirec);
		        	if(changeDir == -1)
		        		perror("Error: ");
		        	else
		        	{
		        		free(cwd);
		        		prevwd = getenv("PWD");
			        	cwd = getcwd(NULL, 0);
			        	setenv("PWD", cwd, 1);
		        		prompt = makepwd(promptArray, homeDirec);
		        	}
	        	}
	        	else if (strcmp(args[1], "-")==0)
	        	{
	        		if(args[2] == NULL)
	        		{
			            if(prevwd == NULL)
			               fprintf(stderr, "Error: no previous directory\n");
			            else
			            {
			                char* hellothere = prevwd;
			                prevwd = getenv("PWD");
			                int changeDir = chdir(hellothere);
			                if(changeDir == -1)
			                    perror("Error: ");
			                else
			                {
			                    free(cwd);
			                    cwd = getcwd(NULL, 0);
			                    setenv("PWD", cwd, 1);
			                    prompt = makepwd(promptArray, hellothere);
			                }
			            }
			        }
			        else
				    	fprintf(stderr, "Error: Too many arguments\n");
		        }
	        	else if (strcmp(args[1], ".") == 0)
		        {
		        	if(args[2] == NULL)
	        		{
			            int changeDir = chdir(".");
			            if(changeDir == -1)
			                perror("Error: ");
			            else
			            {
			                free(cwd);
			                prevwd = getenv("PWD");
			                cwd = getcwd(NULL, 0);
			                setenv("PWD", cwd, 1);
			            	prompt = makepwd(promptArray, cwd);
			            }
			        }
			        else
				    	fprintf(stderr, "Error: Too many arguments\n");
		        }
		        else if (strcmp(args[1], "..") == 0)
		        {
		        	if(args[2] == NULL)
	        		{
			        	int changeDir = chdir("..");
			        	if(changeDir == -1)
			        		perror("Error: ");
			        	else
			        	{
				            free(cwd);
				            prevwd = getenv("PWD");
				        	cwd = getcwd(NULL, 0);
				        	setenv("PWD", cwd, 1);
				        	prompt = makepwd(promptArray, cwd);
				        }
				    }
				    else
				    	fprintf(stderr, "Error: Too many arguments\n");
		        }
		        else
		        {
		        	if(args[2] == NULL)
	        		{
			        	int changeDir = chdir(args[1]);
			        	if(changeDir == -1)
			        		perror("Error: ");
			        	else
			        	{
				            free(cwd);
				            prevwd = getenv("PWD");
				        	cwd = getcwd(NULL, 0);
				        	setenv("PWD", cwd, 1);
				        	prompt = makepwd(promptArray, cwd);
				        }
				    }
				    else
				    	fprintf(stderr, "Error: Too many arguments\n");
				}
	        }
        }
        else if(strcmp(args[0], "alarm") == 0)
        {
        	if(redirect == 0 && superRed == 0 && pipe1 == 0)
        	{
	        	int invalid = 0;
	        	int i = 0;
	        	int num = 0;
	        	while(*(args[1]+i) != '\0')//get num
	        	{
	        		if(*(args[1]+i) < '0' || *(args[1]+i) > '9')
	        			invalid = 1;
	        		else
	        			num = num*10 + *(args[1]+i) - 48;
	        		i++;
	        	}
	        	//handle alarm
	        	if(args[1] != NULL && invalid == 0 && strcmp(args[1], "0") != 0)
	        	{
	        		setenv("ALARM", args[1], 1);
	        		alarm(num);
	        	}
	        	else
	        		fprintf(stderr, "Error: need a number as an argument.\n");
	        }
	        else
	        	fprintf(stderr, "Error: need a number as an argument.\n");
        }
        else
        //executables
        {
        	if(pipe1 == 0 && pipe2 == 0)
        	{
		        if((pid = fork()) == 0)
		        {
		        	executable(redirect, redirects, args, envp, superRed, superRedirect);
			       	exit(0);
			    }
			    else
			    {}
		        wait(&pid);
		    }
		    else if(pipe2 == 1)
		    {
		    	pid_t pid1;
		    	pid_t pid2;
		    	pipe(mypipes);
		    	pipe(mypipes1);
		    	if((pid = fork()) == 0)
		        {
		        	dup2(mypipes[1], STDOUT_FILENO);
		        	close(mypipes[0]);
		        	executable(redirect, redirects, args, envp, superRed, superRedirect);
			       	exit(0);
			    }
			    else{}
			    if((pid1 = fork()) == 0)
			    {
			    	dup2(mypipes[0], STDIN_FILENO);
			    	close(mypipes[1]);
			    	dup2(mypipes1[1], STDOUT_FILENO);
			    	close(mypipes1[0]);
			    	executable(redirect, redirects, args1, envp, superRed, superRedirect);
			    	exit(0);
			    }
			    else{}
			    close(mypipes[0]);
				close(mypipes[1]);
			    if((pid2 = fork()) == 0)
			    {
			    	dup2(mypipes1[0], STDIN_FILENO);
			    	close(mypipes1[1]);
			    	executable(redirect, redirects, args2, envp, superRed, superRedirect);
			    	exit(0);
			    }
			    else{}

				close(mypipes1[0]);
				close(mypipes1[1]);
			    wait(&pid);
			    wait(&pid1);
			    wait(&pid2);
		    }
		    else if(pipe1 == 1)
		    {
		    	pid_t pid1;
		    	pipe(mypipes);
		    	if((pid = fork()) == 0)
		        {
		        	dup2(mypipes[1], STDOUT_FILENO);
		        	close(mypipes[0]);
		        	executable(redirect, redirects, args, envp, superRed, superRedirect);
			       	exit(0);
			    }
			    else{}
			    if((pid1 = fork()) == 0)
			    {
			    	dup2(mypipes[0], STDIN_FILENO);
			    	close(mypipes[1]);
			    	executable(redirect, redirects, args1, envp, superRed, superRedirect);
			    	exit(0);
			    }
			    else{}
			    close(mypipes[0]);
				close(mypipes[1]);
			    wait(&pid);
			    wait(&pid1);
		    }
        }
        for(int i = 0; i < numArgs; i++)
        	args[i] = NULL;
        for(int i = 0; i < numArgs1; i++)
        	args1[i] = NULL;
        for(int i = 0; i < numArgs2; i++)
        	args2[i] = NULL;
        for(int i = 0; i < 4; i++)
	   		redirects[i] = NULL;
	   	for(int i = 0; i < 8; i++)
	   		superRedirect[i] = NULL;
        //printf("%s\n",cmd);
        /* All your debug print statements should use the macros found in debu.h */
        /* Use the `make debug` target in the makefile to run with these enabled. */
        info("Length of command entered: %ld\n", strlen(cmd));
        /* You WILL lose points if your shell prints out garbage values. */
        free(cmd);
    }

    /* Don't forget to free allocated memory, and close file descriptors. */
    free(cwd);
    free(cmd);

    return EXIT_SUCCESS;
}
