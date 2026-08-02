#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main(void)
{
    int fd;

    char message[] = "Hello from user space";

    fd = open("/dev/wait_queue", O_WRONLY);

    if (fd < 0) {
        perror("open");
        return -1;
    }

    write(fd, message, strlen(message) + 1);

    printf("Data written successfully\n");

    close(fd);

    return 0;
}