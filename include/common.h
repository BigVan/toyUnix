#ifndef COMMON_H
#define COMMON_H

typedef int             int32_t;
typedef unsigned int    uint32_t;
typedef short           int16_t;
typedef unsigned short  uint16_t;
typedef char            int8_t;
typedef unsigned char   uint8_t;

// set character to monitor
void     outb(uint16_t port, uint8_t value);

uint8_t  inb(uint16_t port);
uint16_t inw(uint16_t port);

void memset(void *buf, int c, uint32_t n);

#endif