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

#include "client_include.h"

#define BUFF_SIZE 1024

char buffer[BUFF_SIZE];


int connect_to_server(char * ip_address){
    int sockfd, connfd;
	struct sockaddr_in servaddr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1){
		printf("client socket error\n");
		exit(1);
	}

	bzero(&servaddr, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(ip_address);
	servaddr.sin_port = htons(PORT);

	if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))!= 0) {
		printf("client connect error\n");
		exit(1);
	}
    return sockfd;

}

struct window * create_window(int height, int width, int handle){

    struct request * request = (struct request *)malloc(sizeof(struct request));
    request->opcode = CREATE_WINDOW;
	request->num_args = 2;
	request->args[0] = height;
	request->args[1] = width;

	int ret = write(handle, request, sizeof(struct request));
//	int ret = write(handle, "hi", 3);
	printf("write ret is %d\n", ret);
	if(ret < 0){
	perror("ret error\n");
	}
    ret = read(handle, buffer, BUFF_SIZE);    
	printf("read ret is %d\n", ret);
    if(ret < 0 || ret != sizeof(struct response)){
	    printf("inside create window if%d, %d\n", ret, sizeof(struct response));
        return NULL;
    }

	struct response * response = (struct response *)buffer;

    if(response->return_value == 0){

        struct window * new_window = (struct window *)malloc(sizeof(struct window));
        new_window->height = height;
        new_window->width = width;
        new_window->mapped = 0;
        new_window->window_id = response->response[0];
        int key = response->response[1];
        int size = response->response[2];
        new_window->size = size;
        int id = shmget(key, size, 0);
        printf("id is %d\n", id);
        new_window->addr = (char *)shmat(id, 0, 0);
        if(new_window->addr < 0){
            printf("shmat error\n");
        }
        printf("%p\n", new_window->addr);
        return new_window;

    }
    else{
        return NULL;
    }

}

int map_window(struct window * window, int handle){

    struct request * request = (struct request *)malloc(sizeof(struct request));
    request->opcode = MAP_WINDOW;
	request->num_args = 1;
    request->args[0] = window->window_id;

	int ret = write(handle, request, sizeof(struct request));
    if(ret != sizeof(struct request)){
        printf("map_window error");
        return -1;
    }
    ret = read(handle, buffer, BUFF_SIZE);    

    if(ret < 0 || ret != sizeof(struct response)){
        return 0;
    }

	struct response * response = (struct response *)buffer;

    return response->return_value;

}

int unmap_window(struct window * window, int handle){

    struct request * request = (struct request *)malloc(sizeof(struct request));
    request->opcode = UNMAP_WINDOW;
	request->num_args = 1;
    request->args[0] = window->window_id;

	int ret = write(handle, request, sizeof(struct request));
    if(ret != sizeof(struct request)){
        printf("unmap_window error");
        return -1;
    }
    ret = read(handle, buffer, BUFF_SIZE);    

    if(ret < 0 || ret != sizeof(struct response)){
        return 0;
    }

	struct response * response = (struct response *)buffer;

    return response->return_value;

}

int destroy_window(struct window * window, int handle){

    struct request * request = (struct request *)malloc(sizeof(struct request));
    request->opcode = DESTROY_WINDOW;
	request->num_args = 1;
    request->args[0] = window->window_id;

	int ret = write(handle, request, sizeof(struct request));
    if(ret != sizeof(struct request)){
        printf("destroy_window error");
        return -1;
    }
    ret = read(handle, buffer, BUFF_SIZE);    

    if(ret < 0 || ret != sizeof(struct response)){
        return 0;
    }

	struct response * response = (struct response *)buffer;
    if(response->return_value == 0){

    }
    return response->return_value;

}

struct event *  get_current_event(struct window * window, int handle){
    struct request * request = (struct request *)malloc(sizeof(struct request));
    request->opcode = CURRENT_EVENT;
	request->num_args = 1;
    request->args[0] = window->window_id;

	int ret = write(handle, request, sizeof(struct request));
    if(ret != sizeof(struct request)){
        printf("destroy_window error");
        return NULL;
    }
    ret = read(handle, buffer, BUFF_SIZE);    

    if(ret < 0 || ret != sizeof(struct response)){
        return NULL;
    }

	struct response * response = (struct response *)buffer;
    int event_bits = response->response[0];

    if(!(event_bits >> NO_EVENT)){
        int num_responses = response->num_responses;
        struct event * event = (struct event *)malloc(sizeof(struct event));
        event->event_bits = response->response[0];
        if(event_bits >> MOUSE_EVENT){
            event->x = response->response[1];
            event->y = response->response[2];
            event->left_clicked = response->response[3];
            event->right_clicked = response->response[4];
            event->mid_clicked = response->response[5];
        }
        if(event_bits >> KEYBOARD_EVENT){

        }
        return event;
    }else{
        return NULL;
    }
}