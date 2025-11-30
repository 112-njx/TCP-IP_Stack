//包结构的定义
#include <string.h>
#include "xnet_tiny.h"

//协议栈网卡ip地址
static const xipaddr_t netif_ipaddr = XNET_CFG_NETIF_IP;

//(无回报)arp包发送的广播地址
static const uint8_t ether_broadcast[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};


#define min(a,b) (a>b ? b : a)

//解决网络编程和本地数据存储的高低位问题
#define swap_order16(v) ( (((v) & 0xFF) << 8) | (((v) >> 8) & 0xFF) )

//本机MAC地址
static uint8_t netif_mac[XNET_MAC_ADDR_SIZE];

static xnet_packet_t tx_packet,rx_packet;

//本次项目定义一个arp映射表
static xarp_entry_t arp_entry;

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
    return xarp_make_request(&netif_ipaddr);   //初始化成功,向全体网络广播arp包
}

//以太网包发送函数(将ip包或者arp包通过以太网发送出去)
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
            //移除包头,使packet中指针指向的区域是arp包区域。
            remove_header(packet,sizeof (_xether_hdr_t));
            xarp_in(packet);
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

//arp初始化函数
void xarp_init(void){
    arp_entry.state = XARP_ENTRY_FREE;
}

//初始化函数
void xnet_init(void){
    ethernet_init();
    xarp_init();
}

//arp包发送函数
int xarp_make_request(const xipaddr_t* ipaddr){
    xnet_packet_t* packet = xnet_alloc_for_send(sizeof (xarp_packet_t));
    xarp_packet_t* arp_packet = (xarp_packet_t*)packet -> data;

    arp_packet->hw_type = XARP_HW_ENTER;
    arp_packet->pro_type = swap_order16(XNET_PROTOCOL_IP);
    arp_packet->hw_len = XNET_MAC_ADDR_SIZE;
    arp_packet->pro_len = XNET_IPV4_ADDR_SIZE;
    arp_packet->opcode = swap_order16(XARP_REQUEST);
    memcpy(arp_packet->sender_mac,netif_mac,XNET_MAC_ADDR_SIZE);
    memcpy(arp_packet->sender_ip,netif_ipaddr.array,XNET_IPV4_ADDR_SIZE);
    memset(arp_packet->target_mac,0,XNET_MAC_ADDR_SIZE);
    memcpy(arp_packet->target_ip,ipaddr->array,XNET_IPV4_ADDR_SIZE);
    return ethernet_out_to(XNET_PROTOCOL_ARP,ether_broadcast,packet);
}

//当收到arp包时的处理
//包检查:检查数据包格式是否正确
//  判断数据包类型,若为arp请求包,返回响应,表项更新

//arp包响应函数,制作一个响应包通过以太网发出
xnet_err_t xarp_make_response (xarp_packet_t* arp_packet){
    xnet_packet_t* packet = xnet_alloc_for_send(sizeof (xarp_packet_t));
    xarp_packet_t* response_packet = (xarp_packet_t*)packet -> data;

    response_packet->hw_type = XARP_HW_ENTER;
    response_packet->pro_type = swap_order16(XNET_PROTOCOL_IP);
    response_packet->hw_len = XNET_MAC_ADDR_SIZE;
    response_packet->pro_len = XNET_IPV4_ADDR_SIZE;
    response_packet->opcode = swap_order16(XARP_REPLY);
    memcpy(arp_packet->sender_mac,netif_mac,XNET_MAC_ADDR_SIZE);
    memcpy(arp_packet->sender_ip,netif_ipaddr.array,XNET_IPV4_ADDR_SIZE);
    memset(arp_packet->target_mac,0,XNET_MAC_ADDR_SIZE);
    memcpy(arp_packet->target_ip,ipaddr->array,XNET_IPV4_ADDR_SIZE);
    return ethernet_out_to(XNET_PROTOCOL_ARP,ether_broadcast,packet);
}

//arp包处理函数
void xarp_in(xnet_packet_t *packet){
    if (packet->size >= sizeof (xarp_packet_t)){
        xarp_packet_t* arp_packet = (xarp_packet_t*)packet->data;

        //获取arp包是请求包还是响应包
        uint16_t opcode = swap_order16(arp_packet->opcode);

        //进行arp包格式,字段检查1判断硬件类型
        if ((swap_order16(arp_packet->hw_type) != XARP_HW_ENTER) ||
           (arp_packet->hw_len != XNET_MAC_ADDR_SIZE) ||
           (swap_order16(arp_packet->pro_type) != XNET_PROTOCOL_IP) ||
           (arp_packet->pro_len != XNET_IPV4_ADDR_SIZE) || (opcode != XARP_REPLY))
            return;

        //判断传入的ip是否是协议栈的ip
        if (!xipaddr_is_equal_buf(&netif_ipaddr,arp_packet->target_ip)){
            return;
        }

        //
        switch (opcode) {
            case XARP_REQUEST:
                xarp_make_response(arp_packet);
                break;
            case XARP_REPLY:
                break;
        }
    }
}

//轮询网卡数据函数
void xnet_poll(void){
    ethernet_poll();
}









