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
#include "font.h"

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
	while(1){
		struct event * event = get_current_event(window, handle);
		// draw an a
		// for(int i=0;i<8;i++){
		// 	for(int j=0;j<8;j++){
		// 		addr[i * 400 + j] = (1 - a[i * 8 + j]) * ((1 << 31) -1);
		// 	}
		// }
		draw_text(context, "hello world", 0, 0, 0x00000000);
		usleep(1000);
	}
}
