#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <signal.h>

//Message in a message queue
struct msgbuf {
		long mtype;       /* message type, must be > 0 */
		char mtext[100];    /* message data */
};