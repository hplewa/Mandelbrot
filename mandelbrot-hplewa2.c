#include "mandelHeaders-hplewa2.h"

//Children pids
pid_t pid1, pid2;

//Message queue ids
int msgqid1, msgqid2;

//Shared memory id
int shmid;

void sig_handler(int sig){
	if(sig == SIGCHLD){
		int status;
		pid_t childpid;
		while ((childpid = (waitpid(-1, &status, 0)))){
			printf("\n");
			if(WIFEXITED(status)){
				// printf("SIGCHLD: exited normally\n");
				if(childpid == pid1){
					printf("mandelCalc exited normally with status %d\n", WEXITSTATUS(status));
				}
				else if(childpid == pid2){
					printf("mandelDisplay exited normally with status %d\n", WEXITSTATUS(status));
				}
				else{
					printf("SIGCHLD: pid: %d exited normally with status %d\n", childpid, WEXITSTATUS(status));
				}
			}
			else{
				printf("SIGCHLD: pid: %d exited abnormally with status %d\n", childpid, WEXITSTATUS(status));
			}
			return;
		}
	}
	if(sig == SIGINT || sig == SIGSEGV){
		//printf("handler: sig = %d\n", sig);
		//cout << "handler: sig = " << sig << endl;
		msgctl(msgqid1, IPC_RMID, NULL);
		msgctl(msgqid2, IPC_RMID, NULL);
		shmctl(shmid, IPC_RMID, NULL);
		kill(pid1, SIGUSR1);
		int status;
		pid_t childpid = waitpid(pid1, &status, 0); //Wait for first child
		if(WIFEXITED(status)){
			printf("mandelCalc exited with status %d.\n", WEXITSTATUS(status));
		}
		kill(pid2, SIGUSR1);
		//printf("waiting on pid2: %d\n", pid2);
		childpid = waitpid(pid2, &status, 0); //Wait for second child
		if( WIFEXITED(status) ){
			printf("mandelDisplay exited with status %d.\n", WEXITSTATUS(status));
		}
		exit(-1);
	}
}

int main(int argc, char* argv[]) {
	printf("Hubert Plewa: hplewa2: Tuesday 4pm\n");
	// 1. Create pipes
	//Pipes
	int pipe1fd[2];
	int pipe2fd[2];

	if (pipe(pipe1fd) == -1) {
		perror("pipe1\n");
		exit(-1);
	}
	if (pipe(pipe2fd) == -1) {
		perror("pipe2\n");
		exit(-2);
	}

	//Memory Queue Paremeters
	key_t mkey = IPC_PRIVATE;	
	int msgflg= 0600|IPC_CREAT;
	// 2. Create message queues
	if((msgqid1 = msgget( mkey,  msgflg)) < 0){
		perror("msgget1\n");
		exit(-3);
	}
	//printf("msgqid1: %d\n", msgqid1);

	if((msgqid2 = msgget( mkey,  msgflg)) < 0){
		perror("msgget2\n");
		exit(-4);
	}
	//printf("msgqid2: %d\n", msgqid2);

	// 3. Create shared memory
	/*( I believe the sample images are 50 rows by 80 columns, which would require at least 4000 * sizeof( int ) ) */
	//Shared memory parameters
	key_t shmkey = IPC_PRIVATE;
	ssize_t shmsize = 4000 * sizeof(int);
	int shmflg = IPC_CREAT|0600;

	//Create shared memory
	if( (shmid = shmget( shmkey,  shmsize,  shmflg)) < 0){
		perror("shmget");
		exit(-5);
	}
	//printf("shmid: %d\n", shmid);

	// 4. Set SIGCHLD handler
	signal(SIGCHLD, sig_handler);
	signal(SIGINT, sig_handler);
	signal(SIGSEGV, sig_handler);

	pid1 = fork();
	if(pid1 < 0){ //Fork failed
		perror("Fork 1 failed.\n");
		exit(-8);
	}
	if(pid1 == 0){ //Child 1 mandelcalc
		// 1. Close unused pipe ends
		// 2. Redirect stdin and stdout with dup2. ( Close these original pipe ends after they've been copied. )
		dup2(pipe1fd[0], STDIN_FILENO); //r p1
		dup2(pipe2fd[1], STDOUT_FILENO); //w p2
		close(pipe1fd[0]); //close r p1
		close(pipe1fd[1]); //close w p1
		close(pipe2fd[0]); // close r p2
		close(pipe2fd[1]); //close w p2
		//pass in shared memory key, and message queue id

		char* c_shmid = (char*)malloc(sizeof(char)*100);
		//char c_shmid[100];
		sprintf(c_shmid, "%d", shmid);
		char* c_msgqid1 = (char*)malloc(sizeof(char)*100);
		//char c_msgqid1[100];
		sprintf(c_msgqid1, "%d", msgqid1);

		//printf("c_shmid: %s, c_msgqid1: %s, c_msgqid2: %s\n", c_shmid, c_msgqid1, c_msgqid2);
		char* cmd[] = { "./mandelCalc", c_shmid, c_msgqid1, NULL};

		// 3. exec mandelCalc with shmid and msgid as arguments. ( Last step in mandelbrot program. )
		execvp(cmd[0], cmd);
		//execlp("./mandelCalc", c_shmid, c_msgqid1, NULL);
		exit(-9); //exec failed
	}
	else{ //Parent
		pid2 = fork();
		
		if(pid2 < 0){ //Fork 2 failed
			perror( "Fork 2 failed.\n");
			exit(-10);
		}
		if(pid2 == 0){ //Child 2
			// 1. Close unused pipe ends
			// 2. Redirect stdin with dup2. ( Close the original pipe end after it has been copied. )
			dup2(pipe2fd[0], STDIN_FILENO); //r p2
			close(pipe1fd[0]); //close r p1
			close(pipe1fd[1]); //close w p1
			close(pipe2fd[0]); // close r p2
			close(pipe2fd[1]); //close w p2

			char* c_shmid = (char*)malloc(sizeof(char)*100);
			sprintf(c_shmid, "%d", shmid);
			char* c_msgqid1 = (char*)malloc(sizeof(char)*100);
			sprintf(c_msgqid1, "%d", msgqid1);
			char* c_msgqid2 = (char*)malloc(sizeof(char)*100);
			sprintf(c_msgqid2, "%d", msgqid2);
	
			//printf("c_shmid: %s, c_msgqid1: %s, c_msgqid2: %s\n", c_shmid, c_msgqid1, c_msgqid2);
			// 3. exec mandelDisplay with shmid and 2 msgids as arguments. ( Last step in mandelbrot program. )
			char* cmd[] = { "./mandelDisplay", c_shmid, c_msgqid1, c_msgqid2, NULL};
			execvp(cmd[0], cmd);
			exit(-11); //exec failed
		}
		else{ //Parent
			close(pipe1fd[0]); //close r p1
			//close(pipe1fd[1]); //close w p1
			close(pipe2fd[0]); // close r p2
			close(pipe2fd[1]); //close w p2
			int status;
			char buf[100];

			//7. While true:
			while(1){
				// a. Read problem info from keyboard
				char filename[100];
				char xMin[100], xMax[100], yMin[100], yMax[100], nRows[100], nCols[100], maxIters[100];
				printf("Type a filename, (# to exit) > "); scanf("%s", filename);
				//printf("Filename: %s\n", filename);
				// b. If user is not done yet:
				if(strcmp(filename, "#") != 0) {
					// i. Write filename to message queue 2
					struct msgbuf msg;
          msg.mtype = 1;
					
					strcpy(msg.mtext, filename);
					if( msgsnd(msgqid2, (void*) &msg, sizeof(msg.mtext), 0) == -1){
						perror("parent msgsnd msgqid2 failed\n");
						msgctl(msgqid1, IPC_RMID, NULL);
						msgctl(msgqid2, IPC_RMID, NULL);
						shmctl(shmid, IPC_RMID, NULL);
						exit(-12);
					}

					// ii. Write xMin, xMax, yMin, yMax, nRows, nCols, and maxIters to pipe
					printf("Enter the # of rows to display > "); scanf("%s", nRows);
					printf("Enter the # of columns to display > "); scanf("%s", nCols);
					printf("Enter the # of iterations > "); scanf("%s", maxIters);
					printf("Enter an xMin > "); scanf("%s", xMin);
					printf("Enter an xMax > "); scanf("%s", xMax);
					printf("Enter a yMin > "); scanf("%s", yMin);
					printf("Enter a yMax > "); scanf("%s", yMax);

					write(pipe1fd[1], xMin, 100);
					write(pipe1fd[1], xMax, 100);
					write(pipe1fd[1], yMin, 100);
					write(pipe1fd[1], yMax, 100);
					write(pipe1fd[1], nRows, 100);
					write(pipe1fd[1], nCols, 100);
					write(pipe1fd[1], maxIters, 100);
				
					// iii. Listen for done messages from both children
				 	int i;
					for(i = 0; i < 2; i++){
						if( msgrcv(msgqid1, (void*) &msg, 100, 0, 0) == -1){
							perror("parent msgrcv msgqid1 failed\n");
							msgctl(msgqid1, IPC_RMID, NULL);
							msgctl(msgqid2, IPC_RMID, NULL);
							shmctl(shmid, IPC_RMID, NULL);
							exit(-13);
						}
					}
				}
				// c. else:
				else{
					//i. Send SIGUSR1 signals to both children
					//ii. Wait for both children, and report exit codes.
					kill(pid1, SIGUSR1);
					pid_t childpid1 = waitpid(pid1, &status, 0); //Wait for first child

					kill(pid2, SIGUSR1);
					pid_t childpid2 = waitpid(pid2, &status, 0); //Wait for second child

					// //	8. Report final results and exit
					msgctl(msgqid1, IPC_RMID, NULL);
					msgctl(msgqid2, IPC_RMID, NULL);
					shmctl(shmid, IPC_RMID, NULL);
					
					exit(1);
				}
			}
		}
	}
}
