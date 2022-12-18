#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <stdint.h>
#include <sys/shm.h>

#include "list.h"
#include "client_include.h"


int main(int argc, char ** argv){
	int handle = connect_to_server("127.0.0.1");
	struct window * window = create_window(100, 200, handle);
	if(window == NULL){
		exit(1);
	}
	while(1){
		(window->addr)[0] += 1;
		sleep(1);
	}
}