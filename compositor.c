#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <stdint.h>
#include <drm/drm.h>
#include <drm/drm_mode.h>

#include "list.h"
#include "common_include.h"
#include "server_include.h"
#include "display_drm.h"
#include "compositor.h"
#include "requests.h"


extern struct display * display;
extern struct list * window_list;
int window_bar_active[4];
int window_bar_inactive[4];

void * composit(){
	while(1){
		sleep(1);
		 print_list(window_list);
		if(window_list->length == 2){
			 printf("%d\n", (((struct window *)(window_list->head->next->data_ptr))->addr)[0]);
		}
	}
}

void compositor_draw(struct display * display, int fb){

    char * addr = display->fbs[fb]->addr;
    int size = display->fbs[fb]->size;
    // paint each pixel
    memset(addr, 0, size);
    pthread_mutex_lock(&window_list->lock);
    for(struct element * element = window_list->head->next;element != NULL; element = element->next){
    
		struct window * window = ((struct window *)(element->data_ptr));
		int map = window->mapped;
//		printf("%d\n", (((struct window *)(window_list->head->next->data_ptr))->addr)[0]);
		int x = window->x;
		int y = window->y;
		int h = window->height;
		int w = window->width;
		int size = window->size;
//		printf("%d * %d, %d\n", x, y, map);
		char * win_addr = window->addr;
		if(map == 1){
			if(element->next != NULL){
				for(int i=0;i<WINDOW_BAR_HEIGHT;i++){
					for(int j=0;j<4*w;j++){
						int address = (i + (y - WINDOW_BAR_HEIGHT)) * 4 * 800 + (j + x*4);
						if(address >= 0 && address < display->size){
							if(element->next->next == NULL){
									addr[address] = window_bar_active[j%4];
							}
							else{
								addr[address] = window_bar_inactive[j%4];
							}
						}
					}
				}
			}
			for(int i=0;i<h;i++){
				for(int j=0;j<4*w;j++){
					int address = (i + y)*4*800 + (j + x*4);
					if(address >= 0 && address < display->size){
						addr[address] = win_addr[i*4*w + j];
					}
				}
			}
		}
		
    }
   
   pthread_mutex_unlock(&window_list->lock); 
   usleep(1000); 
   drm_page_flip(display->fd, display->fbs[fb]->fb_id, display->crtc_id);

}

void * compositor(){

    fd_set fds;
    int fb = 1;
    int prev_left_clicked = 0;
    FD_ZERO(&fds);
    while(1){
        FD_SET(display->fd, &fds);
		FD_SET(display->mouse_fd, &fds);
		int mouse_fd = display->mouse_fd;
		int display_fd = display->fd;
		int max_fd = mouse_fd > display_fd ? mouse_fd : display_fd;
        int ret = select(max_fd + 1, &fds, NULL, NULL, NULL);
        if(ret < 0){
            perror("select error");
        }
        if(FD_ISSET(display->fd, &fds)){
            char buffer[1024];
            int len = read(display->fd, buffer, 1024);
            int pos = 0;
            while(pos < len){
                struct drm_event * e = (struct drm_event *)&buffer[pos];
                pos += e->length;
                if(e->type == DRM_EVENT_FLIP_COMPLETE){
                    compositor_draw(display, fb);
                    fb ^= 1;
                    // break;
                }
            }
        }
		if(FD_ISSET(mouse_fd, &fds)){
			char mouse_buff[3];
			int ret = read(mouse_fd, mouse_buff, 3);
			int left_clicked = mouse_buff[0]&1;
			int right_clicked = mouse_buff[0]&2;
			int mid_clicked = mouse_buff[0]&4;
			int xflag = 1, yflag = 1;
			if(mouse_buff[1] >> 7 == 1){
				mouse_buff[1] = ~mouse_buff[1] + 1;
				xflag = -1;
			}
			if(mouse_buff[2] >> 7 == 1){
				mouse_buff[2] = ~mouse_buff[2] + 1;
				yflag = -1;
			}
			int change_in_x = xflag * mouse_buff[1];
			int change_in_y = yflag * mouse_buff[2];
			struct mouse_window * mouse_window = (struct mouse_window *)(window_list->tail->data_ptr);
			mouse_window->x += change_in_x;
			mouse_window->y -= change_in_y;
			mouse_window->left_clicked = left_clicked;
			mouse_window->right_clicked = right_clicked;
			mouse_window->mid_clicked = mid_clicked;
			//printf("%d %d change\n", change_in_x, change_in_y);
			
    		pthread_mutex_lock(&window_list->lock);
			if(left_clicked){

				for(struct element * element = window_list->tail->prev; element->prev != NULL; element=element->prev){

					struct window * window = (struct window *)element->data_ptr;
					int ms_x = mouse_window->x;
					int ms_y = mouse_window->y;
					if(ms_x > window->x && ms_x < window->x + window->width && ms_y < window->y && ms_y > window->y - WINDOW_BAR_HEIGHT){
						if(element == window_list->tail->prev){
							window->x += change_in_x;
							window->y -= change_in_y;
							printf("inside selected one\n");
						}else if(!prev_left_clicked){
							struct element * next = element->next;
							element->prev->next = next;
							next->prev = element->prev;
							element->next = window_list->tail;
							element->prev = window_list->tail->prev;
							window_list->tail->prev->next = element;
							window_list->tail->prev = element;
							printf("inside unselected one\n");
						}
					break;
					}
				}
			}
			prev_left_clicked = left_clicked;
    		pthread_mutex_unlock(&window_list->lock);

		}
    }
}

void init_window_bar_colour(){

	window_bar_active[0] = 0xFF;
	window_bar_active[1] = 0x00;
	window_bar_active[2] = 0xFF;
	window_bar_active[3] = 0x00;

	window_bar_inactive[0] = 0xFF;
	window_bar_inactive[1] = 0xFF;
	window_bar_inactive[2] = 0x00;
	window_bar_inactive[3] = 0x00;

}

void decide_inital_coordinates(struct window * window){

	int screen_width = display->fbs[0]->width;
	int screen_height = display->fbs[0]->height;
	int window_width = window->width;
	int window_height = window->height;
	srand(time(NULL));
	int x = rand()%(screen_width - window_width);
	int y = rand()%(screen_height - window_height);
	window->x = x;
	window->y = y;

}

void create_mouse_window(){

	struct mouse_window * new_window = (struct mouse_window *)malloc(sizeof(struct mouse_window));
	new_window->height = MOUSE_HEIGHT;
	new_window->width = MOUSE_WIDTH;
	new_window->mapped = 1;
	new_window->size = 4 * MOUSE_HEIGHT * MOUSE_WIDTH;
	new_window->x = display->width / 2;
	new_window->y = display->height / 2;
	new_window->addr = malloc(new_window->size);
	memset(new_window->addr, 255, new_window->size);
	list_insert(window_list, (uint64_t)(new_window), MOUSE_ID);

}

void init_compositor_globals(){
	init_window_bar_colour();
}