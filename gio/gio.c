
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdint.h>
#include <errno.h>

#include <linux/gpio.h>

#include "gio.h"

#if defined(ENABLE_LOG_TRACE)
  #include "log_trace.h"

  #define TAG_NAME     "DEV_GIO"

  #define lDbg(...)        ltMsg(TAG_NAME, LT_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
  #define lWrn(...)        ltMsg(TAG_NAME, LT_WARN,  __FILE__, __LINE__, __VA_ARGS__)
  #define lErr(...)        ltMsg(TAG_NAME, LT_ERR,   __FILE__, __LINE__, __VA_ARGS__)

  #define hexdump(T, P, S) ltDump(TAG_NAME, LT_DEBUG, __FILE__, __LINE__, P, S, T);

#else
  #define TAG_NAME     "GIO"

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

  void gioHexdump(int line, const char *title, void *pack, int size);

  #define hexdump(T, P, S) fiHexdump(__LINE__, T, P, S)
#endif


void gioHexdump(int line, const char *title, void *pack, int size)
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

void gioDevClose(int fd)
{
    if (fd != -1) {
        close(fd);
    }
}

int gioDevOpen(void)
{
    int fd  = -1;

    fd = open(GIO_DEV, O_RDWR | O_SYNC);
    if (fd == -1) {
        lErr("%s open() failed...", GIO_DEV);
    }

    return fd;
}

int gioShowChipInfo(int fd)
{ 
    int ret = 0;

    struct gpiochip_info cinfo;

    if (fd == -1) {
        lWrn("GIO Device file descriptor invalid!!!");
        ret = -EINVAL;
    }
    else {
        memset(&cinfo, 0, sizeof(struct gpiochip_info));
        // Get GPIO Chip info
        if (ioctl(fd, GPIO_GET_CHIPINFO_IOCTL, &cinfo) == -1) {
            lErr("ioctl(GPIO_GET_CHIPINFO_IOCTL) failed...");
        }
        else {
            lDbg("GPIO Chip Name  : %s", cinfo.name);
            lDbg("          Label : %s", cinfo.label);
            lDbg("          Lines : %d", cinfo.lines);
        }
    }

    return ret; 
} 

int gioShowLineInfo(int fd, int gpio)
{ 
    int ret = 0;
    
    struct gpioline_info linfo;

    if (fd == -1) {
        lWrn("GIO Device file descriptor invalid!!!");
        ret = -EINVAL;
    }
    else {
        // Get GPIO Line info
        memset(&linfo, 0, sizeof(struct gpioline_info));
        linfo.line_offset = gpio;
        if (ioctl(fd, GPIO_GET_LINEINFO_IOCTL, &linfo) == -1) {
            lErr("ioctl(GPIO_GET_LINEINFO_IOCTL) failed...");
        }
        else {
            lDbg("GPIO Line Name     : %s", linfo.name);
            lDbg("          Consumer : %s", linfo.consumer);
            lDbg("          Offset   : %d", linfo.line_offset);
            lDbg("          flags    : 0x%08X", linfo.flags);
        }
    }

    return ret; 
} 

void gioShowHandleReq(struct gpiohandle_request *req)
{ 
    int idx = 0; 

    if (req) {   
        lDbg("GPIO Handle request"); 
        lDbg("   fd          : %d", req->fd); 
        lDbg("   label       : %s", req->consumer_label); 
        lDbg("   flags       : 0x%08X", req->flags); 
        lDbg("   lines       : %d", req->lines); 

        for (idx = 0; idx < req->lines; idx++) { 
            lDbg("   [%d]offset  : %d", idx, req->lineoffsets[idx]); 
            lDbg("   [%d]default : %d", idx, req->default_values[idx]); 
        } 
    } 
} 

int gioRequestInput(int fd, const char *label, int *gpios, size_t size, void *hGpio)
{
    int ret = 0,
        idx = 0;

    struct gpiohandle_request req;

    if (fd == -1) {
        lWrn("GIO Device file descriptor invalid!!!");
        ret = -EINVAL;
    }
    else if (gpios == NULL) { 
        lWrn("GIO numbers is not exist!!!"); 
        ret = -EINVAL;
    } 
    else if (size < 1) { 
        lWrn("GIO numbers size is invalid...%d", (int)size); 
        ret = -EINVAL;
    } 
    else {
        // Get GPIO line handle
        memset(&req, 0, sizeof(struct gpiohandle_request));

        req.lines = (size > GPIOHANDLES_MAX) ? GPIOHANDLES_MAX : size; 
        strcpy(req.consumer_label, label);
        req.flags = GPIOHANDLE_REQUEST_INPUT;

        for (idx = 0; idx < req.lines; idx++) { 
            req.lineoffsets[idx]    = gpios[idx];
            req.default_values[idx] = 0;
        }

        if (ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &req) == -1) {
            lErr("ioctl(GPIO_GET_LINEHANDLE_IOCTL) failed...");
            ret = -EFAULT;
        }
        else {
            gioShowHandleReq(&req); 

            if (hGpio) { 
                memcpy(hGpio, &req, sizeof(struct gpiohandle_request)); 
            } 
        }
    }

    return ret;
}

int gioRequestOutput(int fd, const char *label, int val, int *gpios, size_t size, void *hGpio)
{
    int ret = 0,
        idx = 0;

    struct gpiohandle_request req;

    if (fd == -1) {
        lWrn("GIO Device file descriptor invalid!!!");
        ret = -EINVAL;
    }
    else if (gpios == NULL) { 
        lWrn("GIO numbers is not exist!!!"); 
        ret = -EINVAL;
    } 
    else if (size < 1) { 
        lWrn("GIO numbers size is invalid...%d", (int)size); 
        ret = -EINVAL;
    } 
    else {
        // Get GPIO line handle
        memset(&req, 0, sizeof(struct gpiohandle_request));

        req.lines = (size > GPIOHANDLES_MAX) ? GPIOHANDLES_MAX : size; 
        strcpy(req.consumer_label, label);
        req.flags = GPIOHANDLE_REQUEST_OUTPUT;

        for (idx = 0; idx < req.lines; idx++) { 
            req.lineoffsets[idx]    = gpios[idx];
            req.default_values[idx]   = (val > 0) ? 1 : 0;
        }

        if (ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &req) == -1) {
            lErr("ioctl(GPIO_GET_LINEHANDLE_IOCTL) failed...");
            ret = -EFAULT;
        }
        else {
            gioShowHandleReq(&req); 

            if (hGpio) { 
                memcpy(hGpio, &req, sizeof(struct gpiohandle_request)); 
            } 

            ret = req.fd;
        }
    }

    return ret;
}

int gioGetValue(void *hGpio)
{ 
    int ret = 0, 
        idx = 0;

    struct gpiohandle_data    data;
    struct gpiohandle_request *req = NULL;

    req = (struct gpiohandle_request *)hGpio; 
    if (req == NULL) { 
        lWrn("GPIO Handle is not exist!!!"); 
        ret = -EINVAL;
    } 
    else if (req->fd == -1) {
        lWrn("GPIO Handle Device file descriptor invalid!!!");
        ret = -EINVAL;
    }
    else {
        memset(&data, 0, sizeof(struct gpiohandle_data)); 
        if (ioctl(req->fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data) == -1) {
            lErr("ioctl(GPIOHANDLE_GET_LINE_VALUES_IOCTL) failed...");
            ret = -EFAULT;
        } 
        else { 
            for (idx = 0; idx < req->lines; idx++) { 
                lDbg("GPIO Get [%d]offset(%d), default(%d), value(%d)", 
                    idx, req->lineoffsets[idx], req->default_values[idx], data.values[idx]); 
            }     
        } 
    }

    return ret;
} 

int gioSetValue(void *hGpio, int val)
{ 
    int ret = 0, 
        idx = 0;

    struct gpiohandle_data    data;
    struct gpiohandle_request *req = NULL;

    req = (struct gpiohandle_request *)hGpio; 
    if (req == NULL) { 
        lWrn("GPIO Handle is not exist!!!"); 
        ret = -EINVAL;
    } 
    else if (req->fd == -1) {
        lWrn("GPIO Handle Device file descriptor invalid!!!");
        ret = -EINVAL;
    }
    else {
        memset(&data, 0, sizeof(struct gpiohandle_data)); 
        for (idx = 0; idx < req->lines; idx++) { 
            data.values[idx] = (val > 0) ? 1 : 0; 
        } 

        if (ioctl(req->fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data) == -1) {
            lErr("ioctl(GPIOHANDLE_SET_LINE_VALUES_IOCTL) failed...");
            ret = -EFAULT;
        } 
        else { 
            for (idx = 0; idx < req->lines; idx++) { 
                lDbg("GPIO Set [%d]offset(%d), default(%d), value(%d)", 
                    idx, req->lineoffsets[idx], req->default_values[idx], data.values[idx]); 
            }     
        } 
    }

    return ret;
}

