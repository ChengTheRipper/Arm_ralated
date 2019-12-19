#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>



//second drv

int main(int argc, char **argv)
{
    int fd;
    int cnt = 0;
    unsigned char readbuf[4];


    fd = open("/dev/buttom", O_RDWR);
    if(fd < 0)
        printf("cant open !\n");
    while (1)
    {
        read(fd, readbuf, sizeof(readbuf));
        /* code */
        if(!readbuf[0]||!readbuf[1]||!readbuf[2]||!readbuf[3])
        {
            printf("%04d keypressed! %d %d %d %d\n",cnt++ ,readbuf[0] ,readbuf[1] ,readbuf[2] ,readbuf[3]);
        }
    }
    
    return 0;
}
