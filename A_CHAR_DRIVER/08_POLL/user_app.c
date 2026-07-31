#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<poll.h>
#include<string.h>
#include<errno.h>

int main(void)
{
    int fd, ret;
    char buff[100];

    struct pollfd fds;

    fd = open("/dev/poll_dev",O_RDONLY);
    if(fd<0)
    {
        perror("Open Failed\n");
        return EXIT_FAILURE;
    }
    printf("Device Opened\n");

    /*Now Set poll file Descripter*/
    fds.fd = fd;
    fds.events = POLLIN;
    fds.revents = 0;
    printf("Calling Poll() and waiting for data..\n");

    ret = poll(&fds, 1, -1);
    if(ret<0)
    {
        perror("poll faile\n");
        close(fd);
        return EXIT_FAILURE;
    }
    printf("poll() returned from kernel\n");
    if(fds.revents & POLLIN)
    {
        printf("Device is ready to read\n");
        memset(buff,0,sizeof(buff));
        ret = read(fd, buff,sizeof(buff)-1);
        if(ret<0)
        {
            perror("Read_failed\n");
        }
        else{
            printf("Data received %s",buff);
        }
    }
    if(fds.revents & POLLERR)
    {
        printf("Device Error occured\n");
    }
    if(fds.revents & POLLHUP)
    {
        printf("Device Hang_up occured\n");
    }
    close(fd);
    return EXIT_SUCCESS;
}