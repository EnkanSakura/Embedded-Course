#include "adreader.h"
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

ADReader::ADReader()
{
}

// 读取电压模拟量
int ADReader::readVol()
{
    // 打开AD设备文件
    this->adFileDesc = open("/sys/devices/platform/c0000000.soc/c0053000.adc/iio:device0/in_voltage7_raw", 0);
    if (this->adFileDesc < 0)
    {
        printf("adFile open failed");
        exit(1);
    }
    // 读取电压模拟量到buf
    this->readLen = read(this->adFileDesc, this->buf, sizeof this->buf - 1);
    if (this->readLen > 0)
    {
        this->buf[this->readLen] = '\0';
    }
    close(this->adFileDesc);
    return atoi(buf);
}
