#include <stdio.h>
#include "xnet_tiny.h"

int main (void) {
    //初始化以太网包结构
    xnet_init();
    printf("xnet running\n");

    while (1) {
        xnet_poll();  //轮询网卡数据
    }

    return 0;
}
