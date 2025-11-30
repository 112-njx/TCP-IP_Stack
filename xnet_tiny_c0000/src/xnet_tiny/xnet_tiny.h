#ifndef XNET_TINY_H
#define XNET_TINY_H
#include <stdint.h>

//设置本协议栈虚拟机ip地址
#define XNET_CFG_NETIF_IP     {192,168,254,2};

#define XNET_CFG_PACKET_MAX_SIZE 1516
#endif // XNET_TINY_H

#define XNET_IPV4_ADDR_SIZE 4
#define XNET_MAC_ADDR_SIZE 6

//arp协议表标记状态的宏定义
#define XARP_ENTRY_FREE 0
#define XARP_ENTRY_OK 1

//由于编译器结构体自动补充字段的影响，导致MAC地址
// 和协议部分可能不在连续的内存空间上，使用编译器
// 设置解决这一问题
#pragma park(1)   //编译器不自动填充
typedef enum xnet_protocol_t{
    XNET_PROTOCOL_ARP = 0x0806,
    XNET_PROTOCOL_IP = 0x0800,
}xnet_protocol_t;

//ip地址结构
//union 是一种特殊的数据类型，它允许在同一内存位置存储不同的数据类型。
// 联合体的所有成员共享同一块内存空间，联合体的大小由其最大的成员决定。
typedef union _xipaddr_t {
    uint8_t array[XNET_IPV4_ADDR_SIZE];
    uint32_t addr;
}xipaddr_t;

#define XARP_HW_ENTER   0x1
#define XARP_REQUEST    0x1
#define XARP_REPLY      0x2

//ARP包部分 本机首先发送ARP请求寻找那个机器的ip地址是
// 收数据方的地址，该ip地址主机应答该主机MAC地址

//实际需要考虑的问题:1.网卡的数量不固定 该ip地址网卡
// 可能不工作
//2.IP->MAC可能并不固定，IP可能动态分配网卡

//问题解决：表项需可增删改
//1.动态增加新表项
//2.删除无效旧表项
//3.表项无效或错误的检查

//arp包结构
typedef struct _xarp_packet_t{
    uint16_t hw_type,pro_type;                //硬件类型和协议类型
    uint8_t hw_len,pro_len;                   //硬件地址长 + 协议地址长
    uint16_t opcode;                          //请求/响应
    uint8_t sender_mac[XNET_MAC_ADDR_SIZE];     //发送包硬件地址
    uint8_t sender_ip[XNET_IPV4_ADDR_SIZE];     //发送包协议地址
    uint8_t target_mac[XNET_MAC_ADDR_SIZE];   //接收方硬件地址
    uint8_t target_ip[XNET_IPV4_ADDR_SIZE];   //接收方协议地址
}xarp_packet_t;

//rap包头结构
typedef struct _xarp_entry_t {
    xipaddr_t ipaddr;  //ip address
    uint8_t macaddr[XNET_MAC_ADDR_SIZE];  //mac address
    uint8_t state;   //当前状态
    uint16_t tmo;    //超时/剩余生存时间
    uint8_t retry_cnt;   //重试次数
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

//arp包处理函数
void xarp_in(xnet_packet_t *packet);

xnet_err_t xnet_driver_open(uint8_t* mac_addr);

//发送MAC地址
xnet_err_t xnet_driver_send(xnet_packet_t* packet);

//读取数据包
xnet_err_t xnet_driver_read(xnet_packet_t** packet);














