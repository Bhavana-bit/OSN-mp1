#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

int main(int argc, char *argv[])
{
    int fd;
    char buf[100];
    int before, after;

    // Get initial read count
    before = getreadcount();
    printf("Initial read count: %d bytes\n", before);

    // Open file
    fd = open("testfile.txt", O_RDONLY);
    if (fd < 0)
    {
        printf("Error: could not open testfile.txt\n");
        exit(1);
    }

    // Read 100 bytes
    int n = read(fd, buf, sizeof(buf));
    if (n < 0)
    {
        printf("Error: read failed\n");
        close(fd);
        exit(1);
    }
    close(fd);

    // Get updated read count
    after = getreadcount();
    printf("Read count after reading 100 bytes: %d bytes\n", after);
    printf("Bytes increased: %d\n", after - before);

    exit(0);
}
