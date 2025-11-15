#ifndef XNET_TINY_H
#define XNET_TINY_H
#include <stdint.h>

#define XNET_CFG_PACKET_MAX_SIZE 1516
#endif // XNET_TINY_H
//以太网数据包结构
typedef struct xnet_packet_t {
    uint16_t size;
    uint8_t* data;
    uint8_t payload[XNET_CFG_PACKET_MAX_SIZE];  //单个数据包块的最大长度
}xnet_packet_t;

//以太网接收函数，uint16_t是stdint包里16个字节的int类型
xnet_packet_t* xnet_alloc_for_send(uint16_t data_size);
//同理输出函数
xnet_packet_t* xnet_alloc_for_read(uint16_t data_size);
