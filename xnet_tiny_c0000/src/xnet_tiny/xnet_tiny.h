#ifndef XNET_TINY_H
#define XNET_TINY_H
#include <stdint.h>

#define XNET_CFG_PACKET_MAX_SIZE 1516
#endif // XNET_TINY_H

#define XNET_IPV4_ADDR_SIZE 4
#define XNET_MAC_ADDR_SIZE 6

//arp协议表标记状态的宏定义
#define XARP_ENTRY_FREE 0

//由于编译器结构体自动补充字段的影响，导致MAC地址
// 和协议部分可能不在连续的内存空间上，使用编译器
// 设置解决这一问题
#pragma park(1)   //编译器不自动填充

typedef enum xnet_protocol_t{
    XNET_PROTOCOL_ARP = 0x0806,
    XNET_PROTOCOL_IP = 0x0800,
}xnet_protocol_t;

typedef union _xipaddr_t {
    uint8_t array[XNET_IPV4_ADDR_SIZE];
    uint32_t addr;
}xipaddr_t;

#define XARP_HW_ENTER   0x1
#define XARP_REQUEST    0x1
#define XARP_REPLY      0x2

//arp包结构
typedef struct _xarp_packet_t{
    uint16_t hw_type,pro_type;                //硬件类型和协议类型
    uint8_t hw_len,pro_len;                   //硬件地址长 + 协议地址长
    uint16_t opcode;                          //请求/响应
    uint8_t send_mac[XNET_MAC_ADDR_SIZE];     //发送包硬件地址
    uint8_t send_ip[XNET_IPV4_ADDR_SIZE];     //发送包协议地址
    uint8_t target_mac[XNET_MAC_ADDR_SIZE];   //接收方硬件地址
    uint8_t target_ip[XNET_IPV4_ADDR_SIZE];   //接收方协议地址
}xarp_packet_t;

//表结构:xarp_entry_t
//        ip地址
//        MAC地址
//        当前状态
//        超时/剩余生存时间
//        重试次数
typedef struct _xarp_entry_t {
    xipaddr_t ipaddr;  //ip address
    uint8_t macaddr[XNET_MAC_ADDR_SIZE];  //mac address
    uint8_t state;   //
    uint16_t tmo;
    uint8_t retry_cnt;
}xarp_entry_t;

//定义以太网包头结构
typedef struct _xether_hdr_t{
    uint8_t dest[XNET_MAC_ADDR_SIZE];
    uint8_t src[XNET_MAC_ADDR_SIZE];
    uint16_t protocal;
}_xether_hdr_t;

//枚举类型，用于指示读入数据包和读出数据包的返回错误
typedef enum xnet_err_t {
    XNET_ERR_OK = 0,
    XNET_ERR_IO = -1,
}xnet_err_t;

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

//协议栈的初始化
void xnet_init(void);

//查询
void xnet_poll(void);

//arp初始化函数
void xarp_init(void);

//arp包发送请求函数
int xarp_make_request(const xipaddr_t* ipaddr);

xnet_err_t xnet_driver_open(uint8_t* mac_addr);

//发送MAC地址
xnet_err_t xnet_driver_send(xnet_packet_t* packet);

//读取数据包
xnet_err_t xnet_driver_read(xnet_packet_t** packet);















