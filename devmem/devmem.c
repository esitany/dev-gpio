
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/mman.h>

#include "devmem.h"

#if defined(ENABLE_LOG_TRACE)
  #include "log_trace.h"

  #define TAG_NAME     "DEV_MEM"

  #define lDbg(...)        ltMsg(TAG_NAME, LT_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
  #define lWrn(...)        ltMsg(TAG_NAME, LT_WARN,  __FILE__, __LINE__, __VA_ARGS__)
  #define lErr(...)        ltMsg(TAG_NAME, LT_ERR,   __FILE__, __LINE__, __VA_ARGS__)

  #define hexdump(T, P, S) ltDump(TAG_NAME, LT_DEBUG, __FILE__, __LINE__, P, S, T);

#else
  #define TAG_NAME     "PHYMEM"

  #define lDbg(fmt, ...) { \
    fprintf(stdout, "<%8s|%5d> " fmt "\n", \
            TAG_NAME, __LINE__,  ##__VA_ARGS__); \
    fflush(stdout); \
  }

  #define lWrn(fmt, ...) { \
    fprintf(stdout, "<%8s|%5d> \x1b[33m" fmt "\x1B[0m\n", \
            TAG_NAME, __LINE__,  ##__VA_ARGS__); \
    fflush(stdout); \
  }

  #define lErr(fmt, ...) { \
    fprintf(stdout, "<%8s|%5d> \x1b[31m" fmt "[E=%s(%d)]\x1B[0m\n", \
            TAG_NAME, __LINE__,  ##__VA_ARGS__, strerror(errno), errno); \
    fflush(stdout); \
  }

  void devmemHexdump(int line, const char *title, void *pack, int size);

  #define hexdump(T, P, S) fiHexdump(__LINE__, T, P, S)
#endif


void devmemHexdump(int line, const char *title, void *pack, int size)
{
    int   idx = 0;

    char strTmp[4]    = {"\0"};
    char strAscii[32] = {"\0"};
    char strDump[64]  = {"\0"};
    char *dump        = NULL;

    dump = (char *)pack;
    if ((size > 0) && (pack != NULL)) {
        fprintf(stdout, "<%8s|%5d> ***** %s %d bytes *****\n",
                TAG_NAME, line, (title == NULL) ? "None" : title, size);
        fflush(stdout);

        memset(strDump, 0, 64);
        memset(strAscii, 0, 32);

        for(idx = 0; idx < size; idx++) {
            if    ((0x1F < dump[idx]) && (dump[idx] < 0x7F) ) { strAscii[idx & 0x0F] = dump[idx]; }
            else                                              { strAscii[idx & 0x0F] = 0x2E;
            }

            snprintf(strTmp, 4, "%02X ", (unsigned char)dump[idx]);
            strcat(strDump, strTmp);
            if( (idx != 0) && ((idx & 0x03) == 0x03)) { strcat(strDump, " "); }

            if((idx & 0x0F) == 0x0F) {
                fprintf(stdout, "%16s <0x%04X> %s%s\n", "", (idx & 0xFFF0), strDump, strAscii);
                fflush(stdout);
                memset(strDump, 0, 64);
                memset(strAscii, 0, 32);
            }
        }

        if (((size - 1) & 0x0F) != 0x0F) {
            for(idx = strlen(strDump) ; idx < 52; idx++) {
                strDump[idx] = 0x20;
            }
            fprintf(stdout, "%16s <0x%04X> %s%s\n", "", (size & 0xFFF0), strDump, strAscii);
            fflush(stdout);
        }

        fprintf(stdout, "\n");
        fflush(stdout);
    }
}

void devmemClose(int fd)
{
    if (fd != -1) {
        close(fd);
    }
}

int devmemOpen(void)
{
    int fd  = -1;

    fd = open(DEVMEM_DEV, O_RDWR | O_SYNC);
    if (fd == -1) {
        lErr("%s open() failed...", DEVMEM_DEV);
    }

    return fd;
}

int devmemRead(int fd, unsigned long addr, int type, unsigned long *result)
{
    int ret  = 0;

    unsigned long val  = 0;

    void *mapBase  = NULL,
         *virtAddr = NULL;

    if (fd == -1) {
        lWrn("Illegal device file descriptor!!!");
        ret = -EINVAL;
    }
    else {
        mapBase = mmap(0, DEVMEM_MAP_SIZE, PROT_READ, MAP_SHARED, fd, (off_t)(addr & ~DEVMEM_MAP_MASK));
        if (mapBase == (void *)-1) {
            lErr("mmap() failed...");
            ret = -EFAULT;
        }
        else {
            virtAddr = mapBase + (addr & DEVMEM_MAP_MASK);
            switch( tolower(type) ) {
            case 'b' : val = *((unsigned char  *)virtAddr); break;
            case 'h' : val = *((unsigned short *)virtAddr); break;
            case 'w' :
            default  : val = *((unsigned long  *)virtAddr); break;
            }

            lDbg("Value at address 0x%lX(%p) : 0x%lX", addr, virtAddr, val);

            if (munmap(mapBase, DEVMEM_MAP_SIZE) == -1) {
                lErr("munmap() failed...");
                ret = -EFAULT;
            }
        }
    }

    if (result) {
        *result = val;
    }

    return ret;
}

int devmemWrite(int fd, unsigned long addr, int type, unsigned long val)
{
    int ret  = 0;

    unsigned long result  = 0;

    void *mapBase  = NULL,
         *virtAddr = NULL;

    if (fd == -1) {
        lWrn("Illegal device file descriptor!!!");
        ret = -EINVAL;
    }
    else {
        mapBase = mmap(0, DEVMEM_MAP_SIZE, PROT_READ, MAP_SHARED, fd, (off_t)(addr & ~DEVMEM_MAP_MASK));
        if (mapBase == (void *)-1) {
            lErr("mmap() failed...");
            ret = -EFAULT;
        }
        else {
            virtAddr = mapBase + (addr & DEVMEM_MAP_MASK);
            switch( tolower(type) ) {
            case 'b' :
                *((unsigned char  *)virtAddr) = val;
                result = *((unsigned char  *)virtAddr);
                break;
            case 'h' :
                *((unsigned short *)virtAddr) = val;
                result = *((unsigned short *)virtAddr);
                break;
            case 'w' :
            default  :
                *((unsigned long  *)virtAddr) = val;
                result = *((unsigned long  *)virtAddr);
                break;
            }

            lDbg("Writeen Value(0x%lX) at address 0x%lX(%p) : 0x%lX", val, addr, virtAddr, result);

            if (munmap(mapBase, DEVMEM_MAP_SIZE) == -1) {
                lErr("munmap() failed...");
                ret = -EFAULT;
            }
        }
    }

    return ret;
}

