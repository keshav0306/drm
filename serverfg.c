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
#include <drm/drm.h>
#include <drm/drm_mode.h>

#include "list.h"
#include "common_include.h"
#include "server_include.h"
#include "display_drm.h"

int window_bar_active[4];
int window_bar_inactive[4];

struct display * display;
char file_create_buff[9000000];
struct list * window_list;

int num;
pthread_mutex_t name_of_file_lock;

int window_id;
pthread_mutex_t window_id_lock;

char * make_file(int * num_file){
	pthread_mutex_lock(&name_of_file_lock);
	char buff[30] = COMMON_FILE_NAME;
	char num_buff[11];
	*num_file = num;
	printf("%s\n", COMMON_FILE_NAME);
	sprintf(num_buff, "%d", num++);
	printf("%s\n", num_buff);
	char * combined_buff = (char *)malloc(sizeof(char) * (strlen(buff) + strlen(num_buff) + 1));
	char * cat = strcat(buff, num_buff);
	strcpy(combined_buff, cat);
	printf("%d:%d, %s\n", strlen(buff) + strlen(num_buff) + 1, strlen(combined_buff),combined_buff);
	pthread_mutex_unlock(&name_of_file_lock);
	return combined_buff;
}

struct response * error_response(){

	struct response * response = (struct response *)malloc(sizeof(struct response));
	response->return_value = -2;
	response->num_responses = 0;
	return response;

}

struct response * request_create_window(struct request * request){
	
	int num_args = request->num_args;
	struct response * response;
	if(num_args != 2){
		goto error;
	}
	int height = request->args[0];
	int width = request->args[1];

	int num_file;
	int size = height * width * 4;
	char * name = make_file(&num_file);
	int fd = open(name, O_CREAT | O_RDWR);
	write(fd, file_create_buff, size);
	close(fd);
	printf("name is %s\n", name);
	int key = ftok(name, 0);
    int id = shmget(key, size, 0664 | IPC_CREAT);

	if(id < 0){
		perror("shmget error");
	}
    // shmctl(id, IPC_RMID, NULL);

    char * ptr = shmat(id, 0, 0);

	response = (struct response *)malloc(sizeof(struct response));
	response->return_value = 0;
	response->num_responses = 3;
	pthread_mutex_lock(&window_id_lock);
	response->response[0] = window_id++;
	pthread_mutex_unlock(&window_id_lock);
	response->response[1] = key;
	printf("id is %d\n", id);
	response->response[2] = size;

	struct window * new_window = (struct window *)malloc(sizeof(struct window));
	new_window->height = height;
	new_window->width = width;
	new_window->mapped = 0;
	new_window->size = size;
	new_window->addr = ptr;
	new_window->window_id = response->response[0];

	insert_before_last(window_list, (uint64_t)new_window, new_window->window_id);

	return response;
	error:
	return error_response();
}

void create_mouse_window(){

	struct window * new_window = (struct window *)malloc(sizeof(struct window));
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

int change_window_map_data(struct list * list, int window_id, int map){

	pthread_mutex_lock(&list->lock);
	for(struct element * element = list->head; element != NULL; element = element->next){
		if(element->id == window_id){
			struct window * window = (struct window *)(element->data_ptr);
			window->mapped = map;
			if(map == 1){
				decide_inital_coordinates(window);
			}
		}

	}
	pthread_mutex_unlock(&list->lock);

}

struct response * request_map_window(struct request * request){
	int num_args = request->num_args;
	if(num_args != 1){
		goto error;
	}
	int window_id = request->args[0];
	int ret = change_window_map_data(window_list, window_id, 1);
	if(ret < 0){
		goto error;
	}

	struct response * response = (struct response *)malloc(sizeof(struct response));
	response->return_value = 0;
	response->num_responses = 0;
	return response;

	error:
	return error_response();
}

struct response * request_unmap_window(struct request * request){
	int num_args = request->num_args;
	if(num_args != 1){
		goto error;
	}
	int window_id = request->args[0];
	int ret = change_window_map_data(window_list, window_id, 0);
	if(ret < 0){
		goto error;
	}

	struct response * response = (struct response *)malloc(sizeof(struct response));
	response->return_value = 0;
	response->num_responses = 0;
	return response;

	error:
	return error_response();
}

struct response * request_current_event(struct request * request){
	
}

struct response * request_destroy_window(struct request * request){
	int num_args = request->num_args;
	if(num_args != 1){
		goto error;
	}

	int window_id = request->args[0];
	int ret = list_delete(window_list, window_id);
	if(ret < 0){
		goto error;
	}

	struct response * response = (struct response *)malloc(sizeof(struct response));
	response->return_value = 0;
	response->num_responses = 0;
	return response;

	error:
	return error_response();
}

struct response * handle_request(struct request * request){
	int opcode = request->opcode;
	struct response * response;
	switch(opcode){
		case CREATE_WINDOW:
			response = request_create_window(request);
		break;
		case MAP_WINDOW:
			response = request_map_window(request);
		break;
		case UNMAP_WINDOW:
			response = request_unmap_window(request);
		break;
		case CURRENT_EVENT:
			response = request_current_event(request);
		break;
		case DESTROY_WINDOW:
			response = request_destroy_window(request);
		break;
		default:
			response = error_response();
			break;
	}

	return response;
}

void * handle_the_window(void * args){
	int fd = *((int *)args);
	char * buffer = malloc(sizeof(char) * BUFF_SIZE);
	while(1){
		int length_of_request = read(fd, buffer, BUFF_SIZE);
		if(length_of_request < 0){
			printf("read error\n");
			return 0;
		}
		if(length_of_request < sizeof(struct request)){
			printf("network error\n");
			return 0;
		}
		struct request * request = (struct request *)buffer;
		printf("requested opcode %d\n", request->opcode);
		printf("%d %d\n", request->args[0], request->args[1]);
		struct response * response = handle_request(request);
		printf("respond value %d\n", response->return_value);
		int len = write(fd, response, sizeof(struct response));
		if(len != sizeof(struct response)){
			printf("write error\n");
			return 0;
		}
	}
}

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
						if(element->next->next == NULL){
							addr[(i + (y - WINDOW_BAR_HEIGHT)) * 4 * 800 + (j + x*4)] = window_bar_active[j%4];
						}
						else{
							addr[(i + (y - WINDOW_BAR_HEIGHT)) * 4 * 800 + (j + x*4)] = window_bar_inactive[j%4];
						}
					}
				}
			}
			for(int i=0;i<h;i++){
				for(int j=0;j<4*w;j++){
					addr[(i + y)*4*800 + (j + x*4)] = win_addr[i*4*w + j];
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
			struct window * mouse_window = (struct window *)(window_list->tail->data_ptr);
			mouse_window->x += change_in_x;
			mouse_window->y -= change_in_y;
			//printf("%d %d change\n", change_in_x, change_in_y);

    		pthread_mutex_lock(&window_list->lock);
			if(left_clicked){
<<<<<<< HEAD
				for(struct element * element = window_list->tail->prev; element->prev != NULL; element=element->prev){
=======
				for(struct element * element = window_list->tail->prev; element->prev->prev != NULL && element != window_list->head; element=element->prev){
>>>>>>> 625003197b38a2f0300beaa3b685b8a9cda077c2
					struct window * window = (struct window *)element->data_ptr;
					int ms_x = mouse_window->x;
					int ms_y = mouse_window->y;
					if(ms_x > window->x && ms_x < window->x + window->width && ms_y < window->y && ms_y > window->y - WINDOW_BAR_HEIGHT){
						if(element == window_list->tail->prev){
							window->x += change_in_x;
							window->y -= change_in_y;
							printf("inside selected one\n");
						}else{
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
    		pthread_mutex_unlock(&window_list->lock);

		}
    }
}

int start_the_server(){
	
	pthread_t compositor_thread;
	pthread_create(&compositor_thread, NULL, compositor,  NULL);
	int server_fd, window_connection_fd;
	struct sockaddr_in address, cli;
	int addrlen = sizeof(address);

	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		printf("server socket error");
		exit(1);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);


	if (bind(server_fd, (struct sockaddr*)&address, sizeof(address))< 0){
		perror("server bind error");
		exit(1);
	}

	if (listen(server_fd, MAX_NO_OF_CLIENTS) < 0) {
		printf("server listen error");
		exit(1);
	}
	while(1){

		if ((window_connection_fd = accept(server_fd, (struct sockaddr*)&cli, &addrlen)) < 0){
			perror("server accept error");
			exit(1);
		
		}
		pthread_t handler_thread_id;
		int * id = (int *)malloc(sizeof(int));
		*id = window_connection_fd;
		pthread_create(&handler_thread_id, NULL, handle_the_window, (void *)id);
		
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

void initialize_globals(){
	pthread_mutex_init(&name_of_file_lock, NULL);
	pthread_mutex_init(&window_id_lock, NULL);
	init_window_bar_colour();
	num = 0;
	display = NULL;
	window_id = 1; // window_id = 0 given to root window
	window_list = list_init();
}

void display_init(){

	int fd = open_dev();
//	show_modes(fd);
	int height, width;
//	scanf("%d %d", &height, &width);
	display = choose_mode(fd, 600, 800);
	printf("after choose mode %d %d %d\n", display->crtc_id, display->encoder_id, display->connector_id);
	if(display == NULL){
		return;
	}
	create_framebuffers(display);
//	printf("after create_framebuffer\n");
	int size =display->fbs[0]->size;
	char * addr = display->fbs[0]->addr;
//	memset(addr ,255, size);
	start_display(display);

	int mouse_fd = open("/dev/input/mice", O_RDONLY);
	if(mouse_fd < 0){
		perror("Error opeing mouse\n");
		exit(1);
	}
	display->mouse_fd = mouse_fd;
	create_mouse_window();
	//	printf("after start_display\n");
	//sleep(2);
}


int main(int argc, char const* argv[]){

	initialize_globals();
	sleep(3);
	display_init();

	start_the_server();

}
