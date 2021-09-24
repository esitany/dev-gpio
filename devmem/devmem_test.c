
#include <stdio.h>
#include <stdlib.h>

#include "devmem.h"

void usage(const char *process)
{
		fprintf(stderr, "\nUsage:\t%s { address } [ type [ data ] ]\n", process);
        fprintf(stderr, "\taddress : memory address to act upon\n");
        fprintf(stderr, "\ttype    : access operation type : [b]yte, [h]alfword, [w]ord\n");
        fprintf(stderr, "\tdata    : data to be written\n\n");
}


int main(int argc, char **argv)
{
    int fd   = -1,
        type = 0,
        ret  = 0;

	unsigned long addr = 0,
                  val  = 0;

	if(argc < 2) {
        usage(argv[0]);
    }
    else {
        fd = devmemOpen();
    }

    if (fd != -1) {
	    addr = strtoul(argv[1], 0, 0);

	    if(argc > 2)
		    type = (int)argv[2][0];

	    if(argc > 3) {
		    val = strtoul(argv[3], 0, 0);

            devmemWrite(fd, addr, type, val);
            fprintf(stdout, "devmemWrite()...%d", ret);
        }
        else {
            devmemRead(fd, addr, type, &val);
            fprintf(stdout, "devmemRead()...%d", ret);
        }

        devmemClose(fd);
    }

    return 0;
}

