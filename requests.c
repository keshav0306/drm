#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <stdint.h>
#include <sys/shm.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <linux/input-event-codes.h>
#include <drm/drm.h>
#include <drm/drm_mode.h>
#include <stdio_ext.h>

#include "list.h"
#include "common_include.h"
#include "server_include.h"
#include "display_drm.h"
#include "compositor.h"

extern void open_keyboard();
int num;
pthread_mutex_t name_of_file_lock;

struct kbd_state{
	int num;
	int key;
};

int window_id;
pthread_mutex_t window_id_lock;
char file_create_buff[9000000];
extern struct mouse_window * mouse;
struct kbd_state kbd_state;

extern struct display * display;
extern struct list * window_list;

char * make_file(int * num_file){
	pthread_mutex_lock(&name_of_file_lock);
	char buff[30] = COMMON_FILE_NAME;
	char num_buff[11];
	*num_file = num;
	sprintf(num_buff, "%d", num++);
	char * combined_buff = (char *)malloc(sizeof(char) * (strlen(buff) + strlen(num_buff) + 1));
	char * cat = strcat(buff, num_buff);
	strcpy(combined_buff, cat);
	pthread_mutex_unlock(&name_of_file_lock);
	return combined_buff;
}

struct response * error_response(){

	struct response * response = (struct response *)malloc(sizeof(struct response));
	response->return_value = -2;
	response->num_responses = 0;
	return response;

}

struct response * request_create_window(struct request * request, int fd){
	
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
    int id = shmget(key, size, 0664 | IPC_CREAT);

	if(id < 0){
		perror("shmget error");
		goto error;
	}

    char * ptr = shmat(id, 0, 0);

	response = (struct response *)malloc(sizeof(struct response));
	response->return_value = 0;
	response->num_responses = 3;
	pthread_mutex_lock(&window_id_lock);
	response->response[0] = window_id++;
	pthread_mutex_unlock(&window_id_lock);
	response->response[1] = key;
	response->response[2] = size;

	struct window * new_window = (struct window *)malloc(sizeof(struct window));
	new_window->height = height;
	new_window->width = width;
	new_window->mapped = 0;
	new_window->size = size;
	new_window->addr = ptr;
	new_window->conn_id = fd;
	new_window->shm_id = id;
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
	int x = active_window->x;
	int y = active_window->y;
	int h = active_window->height;
	int w = active_window->width;

	if(window_id == active_window->window_id){
		send_event = 1;
	}
	if(!send_event){
		response->return_value = 0;
		response->num_responses = 0;
	}
	else{
		int mouse_flag = 0;
		int keyboard_flag = 0;
		response->return_value = 0;
		response->num_responses = 1;
		response->response[0] = 0;
		memset(response->response, 0, NUM_ARGS);

		if(mouse->x > x && mouse->x < x + w && mouse->y > y && mouse->y < y + h){
			response->response[0] |= 1 << MOUSE_EVENT;
			response->return_value += 1;
			int start = response->num_responses;
			response->num_responses += 5;
			response->response[1] = mouse->x - x;
			response->response[2] = mouse->y - y;
			response->response[3] = mouse->left_clicked;
			response->response[4] = mouse->right_clicked;
			response->response[5] = mouse->mid_clicked;
			mouse_flag = 1;
		}

		// keyboard event
		uint8_t key[KEY_MAX/8 + 1];
		memset(key, 0, KEY_MAX/8 + 1);
		int ret = ioctl(display->kbd_fd, EVIOCGKEY(sizeof(key)), key);
		if (ret < 0){
			perror("kbd ioctl_error");
			goto out;
		}
		char buff[256];
 	
		// currently transporting only one key if multiple keys are pressed
		for(int i=0;i<KEY_MAX/8;i++){
			if(key[i] != 0){
				int j;
				for(j=0;j<8;j++){
					int left = key[i] / 2;
					if(key[i] & 1 << j){
						break;
					}
		
				}
				int ret = read(display->kbd_fd, buff, 256);
				if(ret < 0){
					perror("read kbd error\n");
					goto error;
				}

				struct input_event * ev = (struct input_event *) buff;
				for(int k=0;k<ret/sizeof(struct input_event);k++){
					if((ev + k) -> type != EV_KEY){
						continue;
					}
					int send = 0;
					if(kbd_state.key != 8*i + j){
						send = 1;
					}
					else{
						if(kbd_state.num >= 2){
							send = 1;
						}
						else{
							kbd_state.num++;
						}
					}
					if((send &&  (ev + k) -> code == 8 * i + j)){
						response->response[6] = (8 * i) + j;
						response->return_value += 1;
						response->num_responses += 1;
						response->response[0] |= 1 << KEYBOARD_EVENT;
						keyboard_flag = 1;
						kbd_state.key = 8*i+j;
						kbd_state.num = 1;
					}
					goto out;
				}
			
			}
		}
	
		if(!mouse_flag && !keyboard_flag){
			response->response[0] |= NO_EVENT;
		}
	
	out:
	pthread_mutex_unlock(&window_list->lock);
	return response;

	error:
	pthread_mutex_unlock(&window_list->lock);
	return error_response();
}
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
	int shm_id = -1;

	pthread_mutex_lock(&window_list->lock);
	for(struct element * element = window_list->head; element != NULL; element = element->next){
		if(element->id == window_id){
			struct window * window = (struct window *)(element->data_ptr);
			shm_id = window->shm_id;
			break;
		}

	}
	pthread_mutex_unlock(&window_list->lock);

    shmctl(shm_id, IPC_RMID, NULL);

	struct response * response = (struct response *)malloc(sizeof(struct response));
	response->return_value = 0;
	response->num_responses = 0;
	return response;

	error:
	return error_response();
}

struct response * handle_request(struct request * request, int fd){
	int opcode = request->opcode;
	struct response * response;
	switch(opcode){
		case CREATE_WINDOW:
			response = request_create_window(request, fd);
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
	kbd_state.key = 0;
	kbd_state.num = 0;
	window_id = 1; // window_id = 0 given to root window
	num = 0;
}
