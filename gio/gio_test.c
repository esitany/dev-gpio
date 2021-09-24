
#include <stdio.h>
#include <stdlib.h>

#include "gio.h"

void usage(const char *process)
{
    fprintf(stdout, "Usage: %s\n", process);
    fprintf(stdout, " GIO Input  : %s i [GPIO LABEL] [GPIO NUM...]\n", process);
    fprintf(stdout, " GIO Output : %s o [GPIO LABEL] [Default Value] [value] [GPIO NUM...]\n", process);

    fflush(stdout);
}

int main(int argc, char **argv)
{
    int fd = -1,
        ret = 0,
        idx = 0;

    int dft  = 0,
        val  = 0,
        mode = 0;

    int gio[32] = { 0, };
    size_t szGIO = 0;

    char ptr[512] = {"\0"};

    char *ch = 0;
    char *label = NULL;

    if (argc > 1) {
        ch = argv[1];

        if ( (ch[0] == 'i') || (ch[0] == 'I')) {
            mode = 'i';
            if (argc < 3) {
                ret = -1;
            }
            else {
                label = argv[2];

                szGIO = argc - 3;
                for (idx = 0; idx < szGIO; idx++) {
                    gio[idx] = (int)strtol(argv[3 + idx], NULL, 10);
                }
            }
        }
        else if ( (ch[0] == 'o') || (ch[0] == 'O')) {
            mode = 'o';
            if (argc < 4) {
                ret = -1;
            }
            else {
                label = argv[2];
                dft   = (int)strtol(argv[3], NULL, 10);
                val   = (int)strtol(argv[4], NULL, 10);
                szGIO = argc - 5;

                for (idx = 0; idx < szGIO; idx++) {
                    gio[idx] = (int)strtol(argv[5 + idx], NULL, 10);
                }
            }
        }
        else {
            ret = -1;
        }
    }
    else {
        ret = -1;
    }

    if (ret >= 0) {
        fd = gioDevOpen();
        switch(mode) {
        case 'i' :
            gioRequestInput(fd, (const char *)label, gio, szGIO, ptr);
            for (idx = 0; idx < szGIO; idx++) {
                gioShowLineInfo(fd, gio[idx]);
            }

            gioGetValue(ptr);
            break;
        case 'o' :
            gioRequestOutput(fd, (const char *)label, dft, gio, szGIO, ptr);
            for (idx = 0; idx < szGIO; idx++) {
                gioShowLineInfo(fd, gio[idx]);
            }


            gioGetValue(ptr);

            gioSetValue(ptr, val);

            for (idx = 0; idx < szGIO; idx++) {
                gioShowLineInfo(fd, gio[idx]);
            }

            gioGetValue(ptr);
            break;
        default  :
            usage(argv[0]);
            break;
        }

        gioDevClose(fd);
    }
    else {
        usage(argv[0]);
    }

    return ret;
}
