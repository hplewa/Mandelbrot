#include "mandelHeaders-hplewa2.h"

int numIterations = 0;

// 7. When SIGUSR1 arrives, exit with non-negative exit code equal to number of images calculated.
void sig_handler(int sig) {
	if(sig == SIGUSR1){
		exit(numIterations);
	}
}

int main(int argc, char* argv[]){
	//fprintf(stderr,"mandelCalc started\n");
	// int i;
	// for(i = 0; i < argc; i++){
	// 	fprintf(stderr,"argv[%d]: %s\n", i, argv[i]);
	// }
// 4. Parse shmid and msgid from command line arguments. ( First step when mandelCalc launches. )
	/*
		argv[0] = "./mandelCalc"
		argv[1] = shmid
		argv[2] = msgid
	*/
	int shmid = atoi(argv[1]);
	int msgqid1 = atoi(argv[2]);

	// 5. Set a signal handler to catch SIGUSR1
	signal(SIGUSR1, sig_handler);

	// 6. While true:
	while(1) {
		// a. read xMin, xMax, yMin, yMax, nRows, nCols, and maxIters from stdin
		char c_xMin[100], c_xMax[100], c_yMin[100], c_yMax[100], c_nRows[100], c_nCols[100], c_maxIters[100];
		double xMin, xMax, yMin, yMax;
		int nRows, nCols, maxIters;
		
		//fprintf(stderr, "Child 1 reading 0.\n");

		//Read in parameters from mandelbrot.
		//Convert from char[] to double or int, respectively
		read(STDIN_FILENO, &c_xMin, 100); xMin = atof(c_xMin);
		//fprintf(stderr, "Child 1, x_xMin: %s xMin: %lf\n",c_xMin, xMin);
		read(STDIN_FILENO, &c_xMax, 100); xMax = atof(c_xMax);
		//fprintf(stderr, "Child 1, xMax: %lf\n", xMax);
		read(STDIN_FILENO, &c_yMin, 100); yMin = atof(c_yMin);
		//fprintf(stderr, "Child 1, yMin: %lf\n", yMin);
		read(STDIN_FILENO, &c_yMax, 100); yMax = atof(c_yMax);
		//fprintf(stderr, "Child 1, yMax: %lf\n", yMax);
		read(STDIN_FILENO, &c_nRows, 100); nRows = atoi(c_nRows);
		//fprintf(stderr, "Child 1, nRows: %d\n", nRows);
		read(STDIN_FILENO, &c_nCols, 100); nCols = atoi(c_nCols);
		//fprintf(stderr, "Child 1, nCols: %d\n", nCols);
		read(STDIN_FILENO, &c_maxIters, 100); maxIters = atoi(c_maxIters);
		//fprintf(stderr, "Child 1, maxIters: %d\n", maxIters);

		//fprintf(stderr, "Child 1 done reading.\n");

		// b. Implement Mandelbrot algorithm to fill shared memory. ( See below. )
		//Access an address in shared memory
		int* shmaddr;
		if( (shmaddr = (int*)shmat(shmid, NULL, 0)) == (int*)-1){
			//If using shared memory fails, delete the memory using ipcrm
			perror("mandelCalc: shmat\n");		
			exit(-1);
		}
		double deltaX = (xMax - xMin) / (nCols - 1);
		double deltaY = (yMax - yMin) / ( nRows -1);

		int r, c, n;
		for(r = 0; r < nRows; r++){
			double Cy = yMin + (r * deltaY);
			for(c = 0; c < nCols; c++){
				double Cx = xMin + (c * deltaX);
				double Zx = 0.0; double Zy = 0.0;
				for(n = 0; n < maxIters; n++){
					if((Zx * Zx) + (Zy * Zy) >= 4.0){
						break;
					}
					double Zx_next = (Zx*Zx) - (Zy*Zy) + Cx;
					double Zy_next = 2.0 * (Zx * Zy) + Cy; 
					Zx = Zx_next;
					Zy = Zy_next;
					numIterations++;
				}
				if( n >= maxIters ){
					//* ( data + R * nCols + C ) 
					//store -1 in data[ r ][ c ]
					*(shmaddr + (r * nCols + c)) = -1;
				}
				else {
					//store n in data[ r ][ c ] 
					*(shmaddr + (r * nCols + c)) = n;
				}
			}
		}

		// c. Write xMin, xMax, yMin, yMax, nRows, and nCols to stdout
		// it will go to mandelDisplay
		write(STDOUT_FILENO, c_xMin, 100);
		write(STDOUT_FILENO, c_xMax, 100);
		write(STDOUT_FILENO, c_yMin, 100);
		write(STDOUT_FILENO, c_yMax, 100);
		write(STDOUT_FILENO, c_nRows, 100);
		write(STDOUT_FILENO, c_nCols, 100);

		// d. Write done message to message queue 1
		struct msgbuf msg;
		msg.mtype = 1;
		strcpy(msg.mtext, "Done.");

		if( msgsnd(msgqid1, (void*) &msg, sizeof(msg.mtext), 0 ) == -1){
			perror("mandelCalc msgsnd msgqid1 failed\n");
			exit(-2);
		}
	}
// 8. ( If any errors occur, exit with a different negative exit code for each potential problem. )
}

