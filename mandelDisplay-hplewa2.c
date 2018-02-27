#include "mandelHeaders-hplewa2.h"

int imagesCalculated = 0;

void sig_handler(int sig) {
	// 7. When SIGUSR1 arrives, exit with non-negative exit code equal to number of images calculated.
	if(sig == SIGUSR1){
		//printf( "mandelCalc: SIGUSR1: %d iterations\n", numIterations);
		exit(imagesCalculated);
	}
}

int main(int argc, char* argv[]){
	// 4. Parse shmid and 2 msgids from command line arguments. ( First step in mandelDisplay. )
	int shmid = atoi(argv[1]);
	int msgqid1 = atoi(argv[2]);
	int msgqid2 = atoi(argv[3]);
	// int i;
	// for(i = 0; i < argc; i++){
	// 	fprintf(stderr,"argv[%d]: %s\n", i, argv[i]);
	// }

	// 5. Set a signal handler to catch SIGUSR1
	signal(SIGUSR1, sig_handler);

	// 6. While true:
	while(1){
		char c_xMin[100], c_xMax[100], c_yMin[100], c_yMax[100], c_nRows[100], c_nCols[100], c_maxIters[100];
		// a. read xMin, xMax, yMin, yMax, nRows, and nCols from stdin
		double xMin, xMax, yMin, yMax;
		int nRows, nCols;
		// cin >> xMin >> xMax >> yMin >> yMax >> nRows >> nCols;
		FILE* infile = fdopen(0, "r");
		if(infile == NULL){
			perror("mandelDisplay can't open 0\n");
			exit(-1);
		}

		//Read in parameters from mandelCalc.
		//Convert from char[] to double or int, respectively
		read(STDIN_FILENO, &c_xMin, 100); xMin = atof(c_xMin);
		//fprintf(stderr, "Child 2, x_xMin: %s xMin: %lf\n",c_xMin, xMin);
		read(STDIN_FILENO, &c_xMax, 100); xMax = atof(c_xMax);
		//fprintf(stderr, "Child 2, xMax: %lf\n", xMax);
		read(STDIN_FILENO, &c_yMin, 100); yMin = atof(c_yMin);
		//fprintf(stderr, "Child 2, yMin: %lf\n", yMin);
		read(STDIN_FILENO, &c_yMax, 100); yMax = atof(c_yMax);
		//fprintf(stderr, "Child 2, yMax: %lf\n", yMax);
		read(STDIN_FILENO, &c_nRows, 100); nRows = atoi(c_nRows);
		//fprintf(stderr, "Child 2, nRows: %d\n", nRows);
		read(STDIN_FILENO, &c_nCols, 100); nCols = atoi(c_nCols);
		//fprintf(stderr, "Child 2, nCols: %d\n", nCols);

		// b. Read filename from message queue 2 and open file. ( If fail, don't save to file. )
		struct msgbuf msg;
		msg.mtype = 1;
		if( msgrcv(msgqid2, (void*) &msg, 100, 0, 0) == -1){
			perror("mandelDisplay msgrcv msgqid2 failed\n");
			exit(-2);
		}
		//printf("child2 read from msgqid2: %s\n", msg.mtext);
		FILE* outfile;
		if((outfile = fopen(msg.mtext, "w")) == NULL){
			fprintf(stderr,"mandelDisplay fopen error, file not found\n");
			//outfile = fdopen(1, "w");
		}
		
		// c. Read data from shared memory and display image on screen
		// d. If file opened properly, save data to a data file. ( May be combined with step c. )
		int* shmaddr;
		if( (shmaddr = (int*)shmat(shmid, NULL, 0)) == (int*)-1){
			//If using shared memory fails, delete the memory using ipcrm
			perror("child2: shmat\n");		
			exit(-2);
		}

		const int nColors = 15;
		char colors[ nColors ] = ".-~:+*%O8&?$@#X";
		int r, c, n;
		for(r = nRows-1; r >= 0; r--){
			for(c = 0; c < nCols; c++){
				n = *(shmaddr + (r*nCols) + c);
				if(n < 0){
					if(outfile != NULL){
						fprintf(outfile, " ");
					}
					printf(" ");
				}
				else{
					if(outfile != NULL){
						fprintf(outfile, "%c", colors[ n % nColors ]);
					}
					printf("%c", colors[ n % nColors ]);
				}
			}
			if(outfile != NULL){
				fprintf(outfile, "\n"); //end of row
			}
			printf("\n"); //end of row
		}
		imagesCalculated++;

		// e. Close file when all data have been written.
		fclose(outfile);
		// f. Write done message to message queue 1.
		strcpy(msg.mtext, "Done.");
		if( msgsnd(msgqid1, (void*) &msg, sizeof(msg.mtext), 0 ) == -1){
			perror("mandelDisplay msgsnd msgqid1 failed\n");
			exit(-3);
		}
	}
	// 8. ( If any errors occur, exit with a different negative exit code for each potential problem. )
}
