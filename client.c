#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <stdint.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "client_include.h"


int main(int argc, char ** argv){
	int handle = connect_to_server("127.0.0.1");
	struct window * window = create_window(400, 400, handle);
	if(window == NULL){
		exit(1);
	}
	map_window(window, handle);
	while(1){
		struct event * event = get_current_event(window, handle);
		if(event){
			if(event->left_clicked){
				unmap_window(window, handle);
			}
		}
		memset(window->addr, 255, window->size);
		sleep(1);
	}
}
