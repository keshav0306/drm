#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#include "compositor.h"

int num;
pthread_mutex_t name_of_file_lock;

int window_id;
pthread_mutex_t window_id_lock;
char file_create_buff[9000000];


extern struct display * display;
extern struct list * window_list;

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
	int num_args = request->num_args;
	if(num_args != 1){
		goto error;
	}
	int window_id = request->args[0];
	struct response * response = (struct response *)malloc(sizeof(struct response));
	
	int send_event = 0;
	pthread_mutex_lock(&window_list->lock);
	struct window * active_window = ((struct window *)(window_list->tail->prev->data_ptr));
	if(window_id == active_window->window_id){
		send_event = 1;
	}
	pthread_mutex_unlock(&window_list->lock);
	if(!send_event){
		response->return_value = 0;
		response->num_responses = 0;
	}
	else{

	}

	return response;

	error:
	return error_response();
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

void init_request_globals(){
    pthread_mutex_init(&name_of_file_lock, NULL);
	pthread_mutex_init(&window_id_lock, NULL);
	window_id = 1; // window_id = 0 given to root window
	num = 0;
}