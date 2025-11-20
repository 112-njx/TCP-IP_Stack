//包结构的定义
#include "xnet_tiny.h"

#define min(a,b) (a>b ? b : a)

static xnet_packet_t tx_packet,rx_packet;
//static修饰以太网数据包结构，后者是接收端和输出端，保证数据在传输时不会改变，是全局变量
xnet_packet_t* xnet_alloc_for_send(uint16_t data_size){
    //数据加上原来的包头
    tx_packet.data = tx_packet.payload + XNET_CFG_PACKET_MAX_SIZE - data_size;
    tx_packet.size = data_size;
    return &tx_packet;  //返回该包起始地址
}

//初始化包
xnet_packet_t* xnet_alloc_for_read(uint16_t data_size){
    rx_packet.data = rx_packet.payload;
    rx_packet.size = data_size;
    return &rx_packet;
}

//读取数据后添加包头函数
static void add_header(xnet_packet_t* packet,uint16_t header_size) {  //以太网数据packet和需要添加的数据header
    packet->data -= header_size;  //packet->data - header_size = packet->data
    packet->size += header_size; //增加包头
}

//移除包头
static void remove_header(xnet_packet_t* packet,uint16_t header_size){
    packet->data += header_size;
    packet->size -= header_size;
}

//截断函数，调用函数时指定截断长度，截断包头数据
static void truncate_packet(xnet_packet_t* packet,uint16_t header_size){
    //定义宏min判断
    packet->size = min(packet->size,header_size);
}


//初始化函数
void xnet_init(void){

}

//轮询网卡数据函数
void xnet_poll(void){

}













