#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>

extern int SrvInit(void);

int main(int argc, char **argv)
{
    int status;

    status = SrvInit();
    if (status)
    {
       sprintf(stderr, "SrvInit: Failed to init SGX services, error code: %d", status);
       return 1;
    }

    return 0;
}
