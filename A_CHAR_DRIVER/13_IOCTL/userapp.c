#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/ioctl.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<fcntl.h>
#include<unistd.h>
#include <stdint.h>

#define WR_VAL _IOW('s',1,int32_t*)
#define RD_VAL _IOR('s',2,int32_t*)

int main()
{
    int fd;
    int32_t value,number;
    printf("***************************\n");
    printf("***************************\n");
    fd = open("/dev/IOCTL_DEVICE", O_RDWR);
    if(fd<0)
    {
        printf("Cannot open device file..\n");
        return 0;
    }
    printf("Enter the value to send\n");
    scanf("%d",&number);

    printf("Writing value to Driver\n");
    ioctl(fd,WR_VAL,(int32_t *)&number);

    printf("Reading VAlue From Driver..\n");
    ioctl(fd,RD_VAL,(int32_t*) &value);
    
    printf("Value is %d\n",value);
    printf("Closing Driver..\n");
    close(fd);
}
