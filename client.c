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
#include "sparkle.h"

int main(int argc, char ** argv){
	int handle = connect_to_server("127.0.0.1");
	struct window * window = create_window(400, 400, handle);
	if(window == NULL){
		exit(1);
	}
	map_window(window, handle);
	struct context * context = new_context(window->height, window->width, window->addr);
	memset(window->addr, 255, window->size);
	while(1){
		struct event * event = get_current_event(window, handle);
		draw_point(context, 2, 2, 0xFF00FF00);
		draw_line(context, 10, 20, 30, 30, 0xFFFF0000);
		if(event){
			if(event->key == 48){
				unmap_window(window, handle);
			}
		}
		
		sleep(1);
	}
}
