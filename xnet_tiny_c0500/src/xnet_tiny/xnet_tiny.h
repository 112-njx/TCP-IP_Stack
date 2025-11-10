/**
 * 用1500行代码从0开始实现TCP/IP协议栈+WEB服务器
 *
 * 本源码旨在用最简单、最易懂的方式帮助你快速地了解TCP/IP以及HTTP工作原理的主要核心知识点。
 * 所有代码经过精心简化设计，避免使用任何复杂的数据结构和算法，避免实现其它无关紧要的细节。
 *
 * 作者：李述铜
 * 微信公众号：01课堂
 * 网址：https://www.yuque.com/lishutong-docs
 *
 * 版权声明：源码仅供学习参考，请勿用于商业产品，不保证可靠性。二次开发或其它商用前请联系作者。
 *
 * 注意：本课程提供的tcp/ip实现很简单，只能够用于演示基本的协议运行机制。我还开发了另一套更加完整的课程，
 * 展示了一个更加完成的TCP/IP协议栈的实现。功能包括：
 * 1. IP层的分片与重组
 * 2. Ping功能的实现
 * 3. TCP的流量控制等
 * 4. 基于UDP的TFTP服务器实现
 * 5. DNS域名接触
 * 6. HTTP服务器
 * 7. 提供socket接口供应用程序使用
 * 8、代码可移植，可移植到arm和x86平台上
 * ..... 更多功能开发中...........
 * 如果你有兴趣的话，请扫仓库中的二维码，或者点击以上面的链接可找到该课程。
 */
#ifndef XNET_TINY_H
#define XNET_TINY_H

#include <stdint.h>

#define XNET_CFG_NETIF_IP               {192, 168, 254, 2}  // 本机网卡IP
#define XNET_CFG_PACKET_MAX_SIZE        1516        // 收发数据包的最大大小
#define XARP_CFG_ENTRY_OK_TMO	        (5)         // ARP表项超时时间
#define XARP_CFG_ENTRY_PENDING_TMO	    (1)          // ARP表项挂起超时时间
#define XARP_CFG_MAX_RETRIES		    4                   // ARP表挂起时重试查询次数
#define XUDP_CFG_MAX_UDP                10                  // 最大支持的UDP连接数

#pragma pack(1)

#define XNET_IPV4_ADDR_SIZE             4           // IP地址长度
#define XNET_MAC_ADDR_SIZE              6           // MAC地址长度

/**
 * 以太网数据帧格式：RFC894
 */
typedef struct _xether_hdr_t {
    uint8_t dest[XNET_MAC_ADDR_SIZE];           // 目标mac地址
    uint8_t src[XNET_MAC_ADDR_SIZE];            // 源mac地址
    uint16_t protocol;                          // 协议/长度
}xether_hdr_t;

#define XARP_HW_ETHER               0x1         // 以太网
#define XARP_REQUEST                0x1         // ARP请求包
#define XARP_REPLY                  0x2         // ARP响应包

typedef struct _xarp_packet_t {
    uint16_t hw_type, pro_type;                 // 硬件类型和协议类型
    uint8_t hw_len, pro_len;                    // 硬件地址长 + 协议地址长
    uint16_t opcode;                            // 请求/响应
    uint8_t sender_mac[XNET_MAC_ADDR_SIZE];     // 发送包硬件地址
    uint8_t sender_ip[XNET_IPV4_ADDR_SIZE];     // 发送包协议地址
    uint8_t target_mac[XNET_MAC_ADDR_SIZE];     // 接收方硬件地址
    uint8_t target_ip[XNET_IPV4_ADDR_SIZE];     // 接收方协议地址
}xarp_packet_t;

typedef struct _xip_hdr_t {
    uint8_t hdr_len : 4;                // 首部长, 4字节为单位
    uint8_t version : 4;                // 版本号
    uint8_t tos;		                // 服务类型
    uint16_t total_len;		            // 总长度
    uint16_t id;		                // 标识符
    uint16_t flags_fragment;            // 标志与分段
    uint8_t ttl;                        // 存活时间
    uint8_t protocol;	                // 上层协议
    uint16_t hdr_checksum;              // 首部校验和
    uint8_t	src_ip[XNET_IPV4_ADDR_SIZE];        // 源IP
    uint8_t dest_ip[XNET_IPV4_ADDR_SIZE];	    // 目标IP
}xip_hdr_t;

typedef struct _xicmp_hdr_t {
    uint8_t type;           // 类型
    uint8_t code;			// 代码
    uint16_t checksum;	    // ICMP报文的校验和
    uint16_t id;            // 标识符
    uint16_t seq;           // 序号
}xicmp_hdr_t;

#pragma pack()

typedef enum _xnet_err_t {
    XNET_ERR_OK = 0,
    XNET_ERR_IO = -1,
    XNET_ERR_NONE = -2,
}xnet_err_t;

/**
 * 网络数据结构
 */
typedef struct _xnet_packet_t{
    uint16_t size;                              // 包中有效数据大小
    uint8_t * data;                             // 包的数据起始地址
    uint8_t payload[XNET_CFG_PACKET_MAX_SIZE];  // 最大负载数据量
}xnet_packet_t;

typedef uint32_t xnet_time_t;           // 时间类型，返回当前系统跑了多少个100ms
const xnet_time_t xsys_get_time(void);
int xnet_check_tmo(xnet_time_t* time, uint32_t sec);

xnet_packet_t * xnet_alloc_for_send(uint16_t data_size);
xnet_packet_t * xnet_alloc_for_read(uint16_t data_size);

xnet_err_t xnet_driver_open (uint8_t * mac_addr);
xnet_err_t xnet_driver_send (xnet_packet_t * packet);
xnet_err_t xnet_driver_read (xnet_packet_t ** packet);

typedef enum _xnet_protocol_t {
    XNET_PROTOCOL_ARP = 0x0806,     // ARP协议
    XNET_PROTOCOL_IP = 0x0800,      // IP协议
    XNET_PROTOCOL_ICMP = 1,         // ICMP协议
}xnet_protocol_t;

/**
 * IP地址
 */
typedef union _xipaddr_t {
    uint8_t array[XNET_IPV4_ADDR_SIZE];     // 以数据形式存储的ip
    uint32_t addr;                          // 32位的ip地址
}xipaddr_t;

#define XARP_ENTRY_FREE		        0       // ARP表项空闲
#define XARP_ENTRY_OK		        1       // ARP表项解析成功
#define XARP_ENTRY_RESOLVING	    2       // ARP表项正在解析
#define XARP_TIMER_PERIOD           1       // ARP扫描周期，1s足够

/**
 * ARP表项
 */
typedef struct _xarp_entry_t {
    xipaddr_t ipaddr;                       // ip地址
    uint8_t	macaddr[XNET_MAC_ADDR_SIZE];    // mac地址
    uint8_t	state;                          // 状态位
    uint16_t tmo;                           // 当前超时
    uint8_t	retry_cnt;                      // 当前重试次数
}xarp_entry_t;

void xarp_init(void);
xnet_err_t xarp_make_request(const xipaddr_t * ipaddr);
void xarp_in(xnet_packet_t * packet);
xnet_err_t xarp_resolve(const xipaddr_t* ipaddr, uint8_t** mac_addr);

void xarp_poll(void);

#define XNET_VERSION_IPV4                   4           // IPV4
#define XNET_IP_DEFAULT_TTL                 64         // 缺省的IP包TTL值
void xip_init(void);
void xip_in(xnet_packet_t * packet);
xnet_err_t xip_out(xnet_protocol_t protocol, xipaddr_t* dest_ip, xnet_packet_t * packet);

#define XICMP_CODE_ECHO_REQUEST             8           // 回显请求
#define XICMP_CODE_ECHO_REPLY               0           // 回显响应
#define XICMP_TYPE_UNREACH                  3           // 目的不可达
#define XICMP_CODE_PORT_UNREACH             3           // 端口不可达
#define XICMP_CODE_PRO_UNREACH              2           // 协议不可达
void xicmp_init(void);
void xicmp_in(xipaddr_t *src_ip, xnet_packet_t * packet);
xnet_err_t xicmp_dest_unreach(uint8_t code, xip_hdr_t *ip_hdr);

typedef struct _xudp_t xudp_t;
typedef xnet_err_t (*xudp_handler_t)(xudp_t * udp, xipaddr_t * src_ip, uint16_t src_port, xnet_packet_t * packet);
struct _xudp_t {
    enum {
        XUDP_STATE_FREE,            // UDP未使用
        XUDP_STATE_USED,            // UDP已使用
    } state;                        // 状态

    uint16_t local_port;            // 本地端口
    xudp_handler_t handler;         // 事件处理回调
};

void xudp_init(void);

void xnet_init (void);
void xnet_poll(void);

#endif // XNET_TINY_H

