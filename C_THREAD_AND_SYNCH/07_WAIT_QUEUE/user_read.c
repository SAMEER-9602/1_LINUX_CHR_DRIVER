#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main(void)
{
    int fd;
    char buffer[100];

    fd = open("/dev/wait_queue", O_RDONLY);

    if (fd < 0) {
        perror("open");
        return -1;
    }

    printf("Calling read()...\n");

    /*
     * This read() goes into the driver's read function.
     * It sleeps because data_available is 0.
     */
    read(fd, buffer, sizeof(buffer));

    printf("Data received: %s\n", buffer);

    close(fd);

    return 0;
}