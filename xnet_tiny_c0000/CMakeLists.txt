cmake_minimum_required(VERSION 3.15)
project(xnet C)

set(CMAKE_C_STANDARD 99)

# 头文件搜索路径
include_directories(
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/src/xnet_app
        ${PROJECT_SOURCE_DIR}/src/xnet_tiny
        ${PROJECT_SOURCE_DIR}/../lib/xnet
        "D:/TCP_IP_Stack/npcap-sdk-1.15/Include"     # Npcap SDK 头文件
)

# Npcap SDK 库目录
link_directories(
        "D:/TCP_IP_Stack/npcap-sdk-1.15/Lib/x64"
)

# 把子目录加进来：这两个通常各自生成 xnet_tiny / xnet_app 静态库
add_subdirectory(src/xnet_tiny)
add_subdirectory(src/xnet_app)

# 把 lib/xnet 作为一个子库加入（里面有 pcap_device.c、enc28j60_device.c 等）
add_subdirectory(
        ${PROJECT_SOURCE_DIR}/../lib/xnet
        ${CMAKE_BINARY_DIR}/lib_xnet_build
)

# 收集本工程自己的源文件
set(SRC
        src/app.c
)

# 生成最终可执行文件
add_executable(${PROJECT_NAME} ${SRC})

# 链接：自家库 + Npcap + Packet + WinSock
target_link_libraries(${PROJECT_NAME}
        xnet_tiny      # 来自 src/xnet_tiny/CMakeLists.txt
        xnet_app       # 来自 src/xnet_app/CMakeLists.txt
        xnet_lib       # 来自 lib/xnet/CMakeLists.txt
        wpcap          # Npcap 主库
        Packet         # Npcap 附属库
        Ws2_32         # WinSock2，Windows 网络库
)
