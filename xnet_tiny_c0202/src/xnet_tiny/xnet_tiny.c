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

static const xipaddr_t netif_ipaddr = XNET_CFG_NETIF_IP;
static const uint8_t ether_broadcast[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static uint8_t netif_mac[XNET_MAC_ADDR_SIZE];                   // mac地址
static xnet_packet_t tx_packet, rx_packet;                      // 接收与发送缓冲区
static xarp_entry_t arp_entry;                                  // 节省内存，只使用一个ARP表项

#define swap_order16(v)   ((((v) & 0xFF) << 8) | (((v) >> 8) & 0xFF))
#define xipaddr_is_equal_buf(addr, buf)      (memcmp((addr)->array, (buf), XNET_IPV4_ADDR_SIZE) == 0)

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

    // 开启抓包工具wireshark，能在窗口发现如下数据包抓取
    // 1	0.000000	Dell_f9:e6:77	Broadcast	ARP	42	ARP Announcement for 192.168.254.2
    return xarp_make_request(&netif_ipaddr);
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
            remove_header(packet, sizeof(xether_hdr_t));
            xarp_in(packet);
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
 * ARP初始化
 */
void xarp_init(void) {
    arp_entry.state = XARP_ENTRY_FREE;
}

/**
 * 生成一个ARP响应
 * @param arp_packet 接收到的ARP请求包
 * @return 生成结果
 */
 xnet_err_t xarp_make_response(xarp_packet_t * arp_packet) {
    xarp_packet_t* response_packet;
    xnet_packet_t * packet = xnet_alloc_for_send(sizeof(xarp_packet_t));

    response_packet = (xarp_packet_t *)packet->data;
    response_packet->hw_type = swap_order16(XARP_HW_ETHER);
    response_packet->pro_type = swap_order16(XNET_PROTOCOL_IP);
    response_packet->hw_len = XNET_MAC_ADDR_SIZE;
    response_packet->pro_len = XNET_IPV4_ADDR_SIZE;
    response_packet->opcode= swap_order16(XARP_REPLY);
    memcpy(response_packet->target_mac, arp_packet->sender_mac, XNET_MAC_ADDR_SIZE);
    memcpy(response_packet->target_ip, arp_packet->sender_ip, XNET_IPV4_ADDR_SIZE);
    memcpy(response_packet->sender_mac, netif_mac, XNET_MAC_ADDR_SIZE);
    memcpy(response_packet->sender_ip, netif_ipaddr.array, XNET_IPV4_ADDR_SIZE);
    return ethernet_out_to(XNET_PROTOCOL_ARP, ether_broadcast, packet);
}

/**
 * 产生一个ARP请求，请求网络指定ip地址的机器发回一个ARP响应
 * @param ipaddr 请求的IP地址
 * @return 请求结果
 */
xnet_err_t xarp_make_request(const xipaddr_t * ipaddr) {
    xarp_packet_t* arp_packet;
    xnet_packet_t * packet = xnet_alloc_for_send(sizeof(xarp_packet_t));

    arp_packet = (xarp_packet_t *)packet->data;
    arp_packet->hw_type = swap_order16(XARP_HW_ETHER);
    arp_packet->pro_type = swap_order16(XNET_PROTOCOL_IP);
    arp_packet->hw_len = XNET_MAC_ADDR_SIZE;
    arp_packet->pro_len = XNET_IPV4_ADDR_SIZE;
    arp_packet->opcode = swap_order16(XARP_REQUEST);
    memcpy(arp_packet->sender_mac, netif_mac, XNET_MAC_ADDR_SIZE);
    memcpy(arp_packet->sender_ip, netif_ipaddr.array, XNET_IPV4_ADDR_SIZE);
    memset(arp_packet->target_mac, 0, XNET_MAC_ADDR_SIZE);
    memcpy(arp_packet->target_ip, ipaddr->array, XNET_IPV4_ADDR_SIZE);
    return ethernet_out_to(XNET_PROTOCOL_ARP, ether_broadcast, packet);
}

/**
 * 更新ARP表项
 * @param src_ip 源IP地址
 * @param mac_addr 对应的mac地址
 */
static void update_arp_entry(uint8_t * src_ip, uint8_t * mac_addr) {
    memcpy(arp_entry.ipaddr.array, src_ip, XNET_IPV4_ADDR_SIZE);
    memcpy(arp_entry.macaddr, mac_addr, 6);
    arp_entry.state = XARP_ENTRY_OK;
}

/**
 * ARP输入处理
 * @param packet 输入的ARP包
 */
void xarp_in(xnet_packet_t * packet) {
    if (packet->size >= sizeof(xarp_packet_t)) {
        xarp_packet_t * arp_packet = (xarp_packet_t *) packet->data;
        uint16_t opcode = swap_order16(arp_packet->opcode);

        // 包的合法性检查
        if ((swap_order16(arp_packet->hw_type) != XARP_HW_ETHER) ||
            (arp_packet->hw_len != XNET_MAC_ADDR_SIZE) ||
            (swap_order16(arp_packet->pro_type) != XNET_PROTOCOL_IP) ||
            (arp_packet->pro_len != XNET_IPV4_ADDR_SIZE)
            || ((opcode != XARP_REQUEST) && (opcode != XARP_REPLY))) {
            return;
        }

        // 只处理发给自己的请求或响应包
        if (!xipaddr_is_equal_buf(&netif_ipaddr, arp_packet->target_ip)) {
            return;
        }

        // 根据操作码进行不同的处理
        switch (swap_order16(arp_packet->opcode)) {
            case XARP_REQUEST:  // 请求，回送响应
                // 在对方机器Ping 自己，然后看wireshark，能看到ARP请求和响应
                // 接下来，很可能对方要与自己通信，所以更新一下
                xarp_make_response(arp_packet);
                update_arp_entry(arp_packet->sender_ip, arp_packet->sender_mac);
                break;
            case XARP_REPLY:    // 响应，更新自己的表
                update_arp_entry(arp_packet->sender_ip, arp_packet->sender_mac);
                break;
        }
    }
}

/**
 * 协议栈的初始化
 */
void xnet_init (void) {
    ethernet_init();
    xarp_init();
}

/**
 * 轮询处理数据包，并在协议栈中处理
 */
void xnet_poll(void) {
    ethernet_poll();
}
