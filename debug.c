#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
//#define MAIN_OFFSET 0xcfc 

int main(int argc,char *argv[] )
{
	pid_t pid;

	if(argc<2){
		fprintf(stderr, "Expected a program name as argument\n");
		return -1;
	}
	
	pid = fork();
	if(pid == 0) run_target(argv[1]);//child
	else if(pid > 0) run_debugger(pid);
	else{
		perror("fork");
		return -1;

	}

	return 0;
}


void run_target(const char* programName){
	printf("target started. will run %s\n", programName);
	
	/*Allow tracing of this process */
	if(ptrace(PTRACE_TRACEME, 0,0,0)<0){
		perror("ptrace");
		return;
	}
	/* replace this process's image with the given program */
	execl(programName, programName,0);
}
 

void run_debugger(pid_t pid){

	int wait_status;
	unsigned icounter =0;
	printf("debugger started\n");

	/*wait for child to stop on its first instruction*/
	wait(&wait_status);

	while(WIFSTOPPED(wait_status)){
		icounter++;
		/*Make the child execute another instruction*/
		if(ptrace(PTRACE_SINGLESTEP, pid,0,0)<0){
			perror("ptrace");
			return;
		}	
	/*wait for child to stop on its next instruction*/
	wait(&wait_status);
	}
	printf("the child executed %u instruction\n",icounter);

}
