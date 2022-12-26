#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

struct termios orig;

void reset(){
    tcsetattr(STDIN_FILENO, TCSANOW, &orig);
}

int main(){
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
        char command[1024];
        sleep(1);
        orig.c_lflag &= ~ECHO;
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
                printf("%s\n", buff);
            }
            if(FD_ISSET(STDIN_FILENO, &fds)){
                read(STDIN_FILENO, command, 1024);
                write(fd, command, 1024);
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
        execlp("/bin/zsh", "/bin/zsh",  NULL);
        printf("failed\n");
    }

}
