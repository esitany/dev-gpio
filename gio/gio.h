/**
* @file gio.h
* @brief This file declares control Linux GPIO
* @author yikim
* @version 1.0
* @date 2021-07-23
*/

#ifndef _LINUX_GPIO_HEADER
#define _LINUX_GPIO_HEADER

#define GIO_DEV  "/dev/gpiochip0"

void gioDevClose(int fd);
int  gioDevOpen(void);

int gioShowChipInfo(int fd);
int gioShowLineInfo(int fd, int gpio);

int gioRequestInput(int fd, const char *label, int *gpios, size_t size, void *hGpio);
int gioRequestOutput(int fd, const char *label, int val, int *gpios, size_t size, void *hGpio);

int gioGetValue(void *hGpio);
int gioSetValue(void *hGpio, int val);

#endif /* _LINUX_GPIO_HEADER */

