# 当前目录：lib/xnet

# 头文件路径
include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# 仅包含 PC 平台需要的 pcap 驱动
set(XNET_LIB_SRC
        pcap_device.c
        pcap_device.h
)

# 创建静态库
add_library(xnet_lib ${XNET_LIB_SRC})
