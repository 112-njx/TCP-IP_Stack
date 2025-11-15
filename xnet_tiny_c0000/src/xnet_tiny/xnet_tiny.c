#include "xnet_tiny.h"

static xnet_packet_t tx_packet,rx_packet;
//static修饰以太网数据包结构，后者是接收端和输出端，保证数据在传输时不会改变，是全局变量
xnet_packet_t* xnet_alloc_for_send(uint16_t data_size){
    //数据加上原来的包头
    tx_packet.data = tx_packet.payload + XNET_CFG_PACKET_MAX_SIZE - data_size;
    tx_packet.size = data_size;
    return &tx_packet;  //返回该包起始地址
}

xnet_packet_t* xnet_alloc_for_read(uint16_t data_size){
    rx_packet.data = rx_packet.payload;
    rx_packet.size = data_size;
    return &rx_packet;
}

//读取数据后添加包头函数
static void add_header(xnet_packet_t* packet,uint16_t header_size) {  //以太网数据packet和需要添加的数据header
    packet->data -= header_size;  //packet->data - header_size = packet->data
    packet->size += header_size;
}