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
	int fd = open("font.h", O_CREAT | O_APPEND | O_RDWR);
	map_window(window, handle);
	chmod("font.h", S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH);
	struct context * context = new_context(window->height, window->width, window->addr);
	memset(window->addr, 255, window->size);
	int * addr = (int *) window->addr;
	int arr[8][8];
	for(int i=0;i<8;i++){
		for(int j=0;j<8;j++){
			arr[i][j] = 0;
		}
	}
	while(1){
		struct event * event = get_current_event(window, handle);
	//	draw_point(context, 2, 2, 0xFF00FF00);
	//	draw_line(context, 100, 20, 200, 100, 0xFFFF0000);
		for(int i=0;i<8;i++){
			for(int j=0;j<8;j++){
				for(int u = 0;u<50;u++){
					for(int v = 0;v<50;v++){
						int origin = 400 * 50 * i + 50 * j;
						addr[origin + u * 400 + v] = (1 - arr[i][j]) * 255;
					}
				}		
			}
		} 
		if(event){
			if(event->event_bits >> MOUSE_EVENT){
				int x = event->x;
				int y = event->y;
				int lclk = event->left_clicked;
				if(lclk){
					arr[y/50][x/50] = 1 - arr[y/50][x/50];
				}
				//printf("%d\n", event->key);
			}
			if(event->event_bits >> KEYBOARD_EVENT){
				if(event->key == 48){
					char begin[17] = "const ";
					char * out = strcat(begin, argv[1]);
					strcpy(begin, out);
					out = strcat(begin, "[64] = {");
					strcpy(begin, out);
					write(fd, begin, strlen(begin));
					for(int i=0;i<8;i++){
						for(int j=0;j<8;j++){
							char buff[5];
							if(i *j != 49)
							sprintf(buff, "%d, ", arr[i][j]);
							else{
							sprintf(buff, "%d", arr[i][j]);
							write(fd, buff, 1);
							break;
							}
							int ret = write(fd, buff, 3);
							printf("%d\n", ret);
						}
					}
					write(fd, "};\n", 3);
				return 0;
				}
			}
		}
		
		usleep(1000);
	}
}
