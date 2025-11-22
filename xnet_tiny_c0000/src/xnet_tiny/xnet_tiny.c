//包结构的定义
#include <string.h>
#include "xnet_tiny.h"

#define min(a,b) (a>b ? b : a)

//解决网络编程和本地数据存储的高低位问题
#define swap_order16(v) ( (((v) & 0xFF) << 8) | (((v) >> 8) & 0xFF) )

//本机MAC地址
static uint8_t netif_mac[XNET_MAC_ADDR_SIZE];

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

//以太网包头结构初始化
static xnet_err_t ethernet_init(void) {
    xnet_err_t err = xnet_driver_open(netif_mac);
    if (err < 0) return err;
    return XNET_ERR_OK;
}

//以太网包头发送函数(将ip包或者arp包通过以太网发送出去)
static xnet_err_t ethernet_out_to(xnet_protocol_t protocol,const uint8_t* mac_addr,xnet_packet_t* packet){
   //函数参数:发送协议 发送mac地址 发送包
   _xether_hdr_t* ether_hdr;  //定义以太网头

    add_header(packet,sizeof (_xether_hdr_t));  //数据包添加以太网头
    ether_hdr = (_xether_hdr_t* )packet->data;
    //复制本机地址和传输目的地地址到包中
    memcpy(ether_hdr->dest,mac_addr,XNET_MAC_ADDR_SIZE);
    memcpy(ether_hdr->src,netif_mac,XNET_MAC_ADDR_SIZE);
    ether_hdr ->protocal = protocol;

    return xnet_driver_send(packet);  //发送包
}

//输出负责对包进行解析
static void ethernet_in(xnet_packet_t* packet){
    _xether_hdr_t * ether_hdr;
    uint16_t protocol;

    //初步判断一下包有没有问题
    if (packet->size <= sizeof (_xether_hdr_t)){
        return;
    }

    ether_hdr = (_xether_hdr_t* )packet->data;
    protocol = swap_order16(ether_hdr ->protocal);
    //判断是ip协议或者ARP协议
    switch (protocol) {
        //涉及到以太网处理数据包大小端的问题！
        //在网络上和在本机上数据方式存储不一样,
        //(1)在网络协议（Ethernet / ARP / IP / TCP/UDP）中：
        //使用大端序，高位字节在前
        //(2)在电脑(x86CPU)中使用:
        //小端序（Little-Endian）高位字节在后。
        //(3)例:0x0800（IP）在网络上线存储是 08 00
        //在电脑中存储是 00 08
        case XNET_PROTOCOL_ARP:
            break;
        case XNET_PROTOCOL_IP:
            break;
    }
}

//轮询观察是否发送数据包
static void ethernet_poll(void) {
    xnet_packet_t* packet;
    if (xnet_driver_read(&packet) == XNET_ERR_OK){
        ethernet_in(packet);  //如果轮询到有数据,调用以太网输入处理函数
    }
}

//初始化函数
void xnet_init(void){
    ethernet_init();
}

//轮询网卡数据函数
void xnet_poll(void){
    ethernet_poll();
}













