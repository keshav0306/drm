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
#include <dirent.h>
#include <drm/drm.h>
#include <drm/drm_mode.h>

#include "list.h"
#include "common_include.h"
#include "server_include.h"
#include "display_drm.h"
#include "compositor.h"
#include "requests.h"


struct display * display;
struct list * window_list;



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
		// printf("requested opcode %d\n", request->opcode);
		// printf("%d %d\n", request->args[0], request->args[1]);
		struct response * response = handle_request(request);
		// printf("respond value %d\n", response->return_value);
		int len = write(fd, response, sizeof(struct response));
		if(len != sizeof(struct response)){
			printf("write error\n");
			return 0;
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



void initialize_globals(){
	
	init_request_globals();
	init_compositor_globals();
	display = NULL;
	window_list = list_init();
}

void open_mouse(){
	int mouse_fd = open("/dev/input/mice", O_RDONLY);
	if(mouse_fd < 0){
		perror("error opeing mouse\n");
		exit(1);
	}

	display->mouse_fd = mouse_fd;
}

void open_keyboard(){
	char * path = "/dev/input/by-id/";
	DIR * d = opendir(path);
	struct dirent * dir;
	if(d){
		while((dir = readdir(d)) != NULL){
			char * name = dir->d_name;
			int length = strlen(name);
			if(length >= 3 && name[length-1] == 'd' && name[length-2] == 'b' && name[length-3] == 'k'){
				char buff[1024];
				strcpy(buff, path);
				strcat(buff, name);
				int fd = open(buff, O_RDONLY);
				if(fd < 0){
					perror("can't access keyboard\n");
					exit(0);
				}
				display->kbd_fd = fd;
				break;
			}
		}
	}
	closedir(d);
}

void display_init(){

	int fd = open_dev();
//	show_modes(fd);
	int height, width;
//	scanf("%d %d", &height, &width);
	display = choose_mode(fd, 600, 800);

	printf("after choose mode %d %d %d\n", display->crtc_id, display->encoder_id, display->connector_id);
	
	if(display == NULL){
		perror("display couldn't be initialized\n");
		return;
	}

	create_framebuffers(display);

	start_display(display);

	open_mouse();

	open_keyboard();

	create_mouse_window();

}


int main(int argc, char const* argv[]){

	initialize_globals();
	sleep(3);
	display_init();

	start_the_server();

}
