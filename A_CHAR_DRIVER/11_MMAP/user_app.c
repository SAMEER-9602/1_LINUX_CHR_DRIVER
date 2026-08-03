#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#define DEVICE_FILE "/dev/mmap_ioctl_device"

#define BUFFER_SIZE 4096

#define MMAP_IOCTL_MAGIC 'M'
#define MMAP_TX_READY _IO(MMAP_IOCTL_MAGIC, 1)

int main()
{
    int fd;

    char *mapped_memory;

    const char *tx_data =
        "Line 01: This is TX data sent using mmap.\n"
        "Line 02: The user application opens the device.\n"
        "Line 03: The mmap system call creates a mapping.\n"
        "Line 04: The driver maps one kernel memory page.\n"
        "Line 05: The application receives a virtual address.\n"
        "Line 06: The application writes through that address.\n"
        "Line 07: No write system call is used here.\n"
        "Line 08: The data is stored in BBB system RAM.\n"
        "Line 09: The driver and application share the memory.\n"
        "Line 10: User and kernel virtual addresses differ.\n"
        "Line 11: The underlying memory page is shared.\n"
        "Line 12: mmap can avoid an additional data copy.\n"
        "Line 13: This example uses one 4 KB page.\n"
        "Line 14: The paragraph is written by user space.\n"
        "Line 15: ioctl notifies the driver after writing.\n"
        "Line 16: The driver receives the TX ready event.\n"
        "Line 17: The driver prints the received paragraph.\n"
        "Line 18: The mapping is removed using munmap.\n"
        "Line 19: The device is closed at the end.\n"
        "Line 20: mmap and ioctl communication is complete.\n";

    fd = open(DEVICE_FILE, O_RDWR);

    if (fd < 0)
    {
        perror("open");

        return 1;
    }

    mapped_memory = mmap(NULL, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (mapped_memory == MAP_FAILED)
    {
        perror("mmap");

        close(fd);

        return 1;
    }

    memset(mapped_memory, 0, BUFFER_SIZE);

    strcpy(mapped_memory, tx_data);

    printf("TX data written into mapped memory\n");

    printf("\n%s\n", mapped_memory);

    if (ioctl(fd, MMAP_TX_READY) < 0)
    {
        perror("ioctl");

        munmap(mapped_memory, BUFFER_SIZE);

        close(fd);

        return 1;
    }

    printf("TX ready notification sent to driver\n");

    munmap(mapped_memory, BUFFER_SIZE);

    close(fd);

    return 0;
}