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
#include <string.h>
#include "xnet_tiny.h"

#define min(a, b)               ((a) > (b) ? (b) : (a))

static uint8_t netif_mac[XNET_MAC_ADDR_SIZE];                   // mac地址
static xnet_packet_t tx_packet, rx_packet;                      // 接收与发送缓冲区

#define swap_order16(v)   ((((v) & 0xFF) << 8) | (((v) >> 8) & 0xFF))

/**
 * 分配一个网络数据包用于发送数据
 * @param data_size 数据空间大小
 * @return 分配得到的包结构
 */
xnet_packet_t * xnet_alloc_for_send(uint16_t data_size) {
    // 从tx_packet的后端往前分配，因为前边要预留作为各种协议的头部数据存储空间
    tx_packet.data = tx_packet.payload + XNET_CFG_PACKET_MAX_SIZE - data_size;
    tx_packet.size = data_size;
    return &tx_packet;
}

/**
 * 分配一个网络数据包用于读取
 * @param data_size 数据空间大小
 * @return 分配得到的数据包
 */
xnet_packet_t * xnet_alloc_for_read(uint16_t data_size) {
    // 从最开始进行分配，用于最底层的网络数据帧读取
    rx_packet.data = rx_packet.payload;
    rx_packet.size = data_size;
    return &rx_packet;
}

/**
 * 为发包添加一个头部
 * @param packet 待处理的数据包
 * @param header_size 增加的头部大小
 */
static void add_header(xnet_packet_t *packet, uint16_t header_size) {
    packet->data -= header_size;
    packet->size += header_size;
}

/**
 * 为接收向上处理移去头部
 * @param packet 待处理的数据包
 * @param header_size 移去的头部大小
 */
static void remove_header(xnet_packet_t *packet, uint16_t header_size) {
    packet->data += header_size;
    packet->size -= header_size;
}

/**
 * 将包的长度截断为size大小
 * @param packet 待处理的数据包
 * @param size 最终大小
 */
static void truncate_packet(xnet_packet_t *packet, uint16_t size) {
    packet->size = min(packet->size, size);
}

/**
 * 以太网初始化
 * @return 初始化结果
 */
static xnet_err_t ethernet_init (void) {
    xnet_err_t err = xnet_driver_open(netif_mac);
    if (err < 0) return err;

    return XNET_ERR_OK;
}

/**
 * 发送一个以太网数据帧
 * @param protocol 上层数据协议，IP或ARP
 * @param mac_addr 目标网卡的mac地址
 * @param packet 待发送的数据包
 * @return 发送结果
 */
static xnet_err_t ethernet_out_to(xnet_protocol_t protocol, const uint8_t *mac_addr, xnet_packet_t * packet) {
    xether_hdr_t* ether_hdr;

    // 添加头部
    add_header(packet, sizeof(xether_hdr_t));
    ether_hdr = (xether_hdr_t*)packet->data;
    memcpy(ether_hdr->dest, mac_addr, XNET_MAC_ADDR_SIZE);
    memcpy(ether_hdr->src, netif_mac, XNET_MAC_ADDR_SIZE);
    ether_hdr->protocol = swap_order16(protocol);

    // 数据发送
    return xnet_driver_send(packet);
}

/**
 * 以太网数据帧输入输出
 * @param packet 待处理的包
 */
static void ethernet_in (xnet_packet_t * packet) {
    // 至少要比头部数据大
    if (packet->size <= sizeof(xether_hdr_t)) {
        return;
    }

    // 往上分解到各个协议处理
    xether_hdr_t* hdr = (xether_hdr_t*)packet->data;
    switch (swap_order16(hdr->protocol)) {
        case XNET_PROTOCOL_ARP:
            break;
        case XNET_PROTOCOL_IP: {
            break;
        }
    }
}

/**
 * 查询网络接口，看看是否有数据包，有则进行处理
 */
static void ethernet_poll (void) {
    xnet_packet_t * packet;

    if (xnet_driver_read(&packet) == XNET_ERR_OK) {
        // 正常情况下，在此打个断点，全速运行
        // 然后在对方端ping 192.168.254.2，会停在这里
        ethernet_in(packet);
    }
}

/**
 * 协议栈的初始化
 */
void xnet_init (void) {
    ethernet_init();
}

/**
 * 轮询处理数据包，并在协议栈中处理
 */
void xnet_poll(void) {
    ethernet_poll();
}
