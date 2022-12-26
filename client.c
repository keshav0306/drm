#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <stdint.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
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
	unsigned int * addr = (unsigned int *) window->addr;
	char buffer[1024] = "hello world";
	int i = strlen(buffer);
	while(1){
		struct event * event = get_current_event(window, handle);
		draw_text(context, buffer, strlen(buffer), 0, 0, 0x00000000);
		if(event->event_bits & 1 << KEYBOARD_EVENT){
			char c = to_char(event->key);
			if(c != 0 && i < 1024){
				buffer[i++] = c;
				buffer[i] = 0;
			}
		}
		usleep(1000);
	}
}
