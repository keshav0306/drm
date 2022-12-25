#define _XOPEN_SOURCE 600

#include <sys/ioctl.h>
#include <termios.h>
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
#include <sys/select.h>

#include "list.h"
#include "client_include.h"
#include "sparkle.h"

struct termios orig;

void reset(){
    tcsetattr(STDIN_FILENO, TCSANOW, &orig);
}

int main(){

  
    // opening the pty

    int fd = posix_openpt(O_RDWR | O_NOCTTY);
    if(fd < 0){
        perror("master open error\n");
        exit(1);
    }
    if(grantpt(fd) < 0){
        perror("grantpt error");
        close(fd);
        exit(1);
    }
    if(unlockpt(fd) < 0){
        perror("unlockpt error");
        close(fd);
        exit(1);
    }
    char * name = ptsname(fd);
    if(!name){
        perror("ptsname error");
        close(fd);
        exit(1);
    }
    
    char buff[1024];
    strcpy(buff, name);
    struct winsize ws;

    tcgetattr(STDIN_FILENO, &orig);
    ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);

    int pid = fork();
    if(pid < 0){
        perror("fork error");
        close(fd);
        exit(1);
    }
    else if(pid != 0){
        // master

        // setting up the connection with the display manager

        int handle = connect_to_server("127.0.0.1");
        struct window * window = create_window(400, 400, handle);
        if(window == NULL){
            exit(1);
        }
        map_window(window, handle);
        struct context * context = new_context(window->height, window->width, window->addr);
        memset(window->addr, 255, window->size);
        unsigned int * addr = (unsigned int *) window->addr;

        char buffer[1024] = {0};
        char command[1024];
        sleep(1);
        // orig.c_lflag &= ~ECHO;
        tcsetattr(STDIN_FILENO, TCSANOW, &orig);
        atexit(reset);
        fd_set fds;
        FD_ZERO(&fds);
        while(1){
            FD_SET(fd, &fds);
            FD_SET(STDIN_FILENO, &fds);
            int ret = select(fd + 1, &fds, NULL, NULL, NULL);
            if(FD_ISSET(fd, &fds)){
                int ret = read(fd, buff, 1024);
                if(ret < 0){
                    exit(0);
                }
                strcat(buffer, buff);
                memset(window->addr, 255, window->size);
		        draw_text(context, buffer, 0, 0, 0x00000000);

            }
            if(FD_ISSET(STDIN_FILENO, &fds)){
                struct event * event = get_current_event(window, handle);
                char c;
		        if(event->event_bits & 1 << KEYBOARD_EVENT){
			        c = to_char(event->key);
                    if(c != 0){
                        write(fd, &c, 1);
                    }
		        }
                // read(STDIN_FILENO, command, 1024);s
            }
        
        }
    }
    else{
        // slave
        if(setsid() < 0){
            perror("setsid error");
            close(fd);
            exit(1);
        }
        close(fd);
        int sfd = open(buff, O_RDWR);
        if(sfd < 0){
            perror("slave open error");
            exit(1);
        }
        // orig.c_lflag &= ~ECHO;
        tcsetattr(sfd, TCSANOW, &orig);
        ioctl(sfd, TIOCSWINSZ, &ws);

        dup2(sfd, 0);
        dup2(sfd, 1);
        dup2(sfd, 2);
        execlp("/bin/bash", "/bin/bash",  NULL);
        printf("failed\n");
    }

}