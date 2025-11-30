/**
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
 */
#include "pcap_device.h"
#include "xnet_tiny.h"

static pcap_t* pcap;
const char * ip_str = "192.168.254.1";   //本机ip地址
const char my_mac_addr[] = {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88};

//初始化驱动设备层
//设置本机MAC地址
//错误码,操作系统内部使用
xnet_err_t xnet_driver_open(uint8_t* mac_addr) {
    memcpy(mac_addr,my_mac_addr,sizeof (my_mac_addr));
    //pcap_device_open()传入
    // const char *ip, const uint8_t *mac_addr, uint8_t poll_mode
    // 物理机ip地址，本机mac地址，查询模式轮询/中断
     pcap = pcap_device_open(ip_str,my_mac_addr,1);
     if (pcap == (pcap_t*) 0){
         exit(-1);
     }
    return XNET_ERR_OK;
}

//发送MAC地址
xnet_err_t xnet_driver_send(xnet_packet_t* packet){
    return pcap_device_send(pcap,packet->data,packet->size);
}

//读取数据包
xnet_err_t xnet_driver_read(xnet_packet_t** packet){
    uint16_t size;
    xnet_packet_t * r_packet = xnet_alloc_for_read(XNET_CFG_PACKET_MAX_SIZE);
    //先分配最大空间
    size = pcap_device_read(pcap,r_packet->data,XNET_CFG_PACKET_MAX_SIZE);
    if (size) {
        r_packet->size = size;
        *packet = r_packet;
        return XNET_ERR_OK;
    }
    return XNET_ERR_IO;
}

//以太网MAC地址六个字节
//以太网数据包结构:
//原MAC地址 6B   目的MAC地址 6B
//上层协议类型 2B (数据负载的格式以及哪种协议处理)  数据负载(46-1500B)


















