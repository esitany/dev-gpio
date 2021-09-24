/**
* @file devmem.h
* @brief This file declares control Linux physical memory
* @author yikim
* @version 1.0
* @date 2021-07-23
*/

#ifndef _LINUX_PHYSICAL_MEMORY_HEADER
#define _LINUX_PHYSICAL_MEMORY_HEADER

#define DEVMEM_DEV  "/dev/mem"

#define DEVMEM_MAP_SIZE    4096UL
#define DEVMEM_MAP_MASK    (DEVMEM_MAP_SIZE - 1)

int  devmemOpen(void);
void devmemClose(int fd);

int devmemRead(int fd, unsigned long addr, int type, unsigned long *result);
int devmemWrite(int fd, unsigned long addr, int type, unsigned long val);


#endif /* _LINUX_PHYSICAL_MEMORY_HEADER */

