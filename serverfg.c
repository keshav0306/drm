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
#include "common_include.h"
#include "server_include.h"
#include "display_drm.h"

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
	sprintf(num_buff, "%d", num++);
	char * combined_buff = (char *)malloc(sizeof(char) * (strlen(buff) + strlen(num_buff) + 1));
	combined_buff = strcat(buff, num_buff);
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
	int key = ftok(name, 0);
    int id = shmget(key, size, 0777 | IPC_CREAT);

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

	list_insert(window_list, (uint64_t)new_window, new_window->window_id);

	return response;
	error:
	return error_response();
}

int change_window_map_data(struct list * list, int window_id, int map){

	pthread_mutex_lock(&list->lock);
	for(struct element * element = list->head; element != NULL; element = element->next){
		if(element->id == window_id){
			struct window * window = (struct window *)(element->data_ptr);
			window->mapped = map;
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
			exit(1);
		}
		if(length_of_request < sizeof(struct request)){
			printf("network error\n");
			exit(1);
		}
		struct request * request = (struct request *)buffer;
		printf("requested opcode %d\n", request->opcode);
		struct response * response = handle_request(request);
		printf("respond value %d\n", response->return_value);
		int len = write(fd, response, sizeof(struct response));
		if(len != sizeof(struct response)){
			printf("write error\n");
			exit(1);
		}
	}
}

void * composit(){
	while(1){
		sleep(1);
		// print_list(window_list);
		if(window_list->length == 2){
			 printf("%d\n", (((struct window *)(window_list->head->next->data_ptr))->addr)[0]);
		}
	}
}

void compositor_draw(struct display * display, int fb){

    char * addr = display->fbs[fb]->addr;
    int size = display->fbs[fb]->size;
    // paint each pixel 
    drm_page_flip(display->fd, display->fbs[fb]->fb_id, display->crtc_id);

}

void * compositor(){

    fd_set fds;
    int fb = 1;
    FD_ZERO(&fds);
    while(1){
        FD_SET(display->fd, &fds);
        int ret = select((display->fd) + 1, &fds, NULL, NULL, NULL);
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
                    fb ~= fb;
                    break;
                }
            }
        }
    }
}

int start_the_server(){
	
	pthread_t compositor;
	pthread_create(&compositor, NULL, compositor,  NULL);
	int server_fd, window_connection_fd;
	struct sockaddr_in address;
	int addrlen = sizeof(address);

	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		printf("server socket error");
		exit(1);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);


	if (bind(server_fd, (struct sockaddr*)&address, sizeof(address))< 0){
		printf("server bind error");
		exit(1);
	}

	if (listen(server_fd, MAX_NO_OF_CLIENTS) < 0) {
		printf("server listen error");
		exit(1);
	}

	while(1){

		if ((window_connection_fd = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0){
			printf("server accept error");
			exit(1);
		}

		pthread_t handler_thread_id;
		int * id = (int *)malloc(sizeof(int));
		*id = window_connection_fd;
		pthread_create(&handler_thread_id, NULL, handle_the_window, (void *)id);
	}

}

void initialize_globals(){
	pthread_mutex_init(&name_of_file_lock, NULL);
	pthread_mutex_init(&window_id_lock, NULL);
	num = 0;
	display = NULL;
	window_id = 1; // window_id = 0 given to root window
	window_list = list_init();
}

void display_init(){

	int fd = open_dev();
	show_modes(fd);
	int height, width;
	scanf("%d %d", &height, &width);
	display = choose_mode(fd, height, width);
	create_framebuffers(display);
	start_display(display);

}


int main(int argc, char const* argv[]){

	initialize_globals();

	display_init();

	start_the_server();

}