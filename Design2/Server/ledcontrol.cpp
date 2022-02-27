#include "ledcontrol.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <QDebug>

// @brief LED控制类
LEDControl::LEDControl(int no)
{
    this->ledNo = no;
    initGPIO();               // 初始化LED对应的GPIO引脚
    this->setLedStatus(true); // 默认高电平，灭
}

// 初始化LED对应的GPIO引脚
void LEDControl::initGPIO()
{
    int fd;
    char buf[5];
    char gpioExportPath[30] = "/sys/class/gpio/export";
    // 配置GPIO
    fd = open(gpioExportPath, O_WRONLY);
    if (fd < 0)
    {
        printf("gpioExportPath:%s\topen failed", gpioExportPath);
        exit(1);
    }
    sprintf(buf, "%2d", this->ledNo);
    write(fd, buf, 2);
    close(fd);

    // 设置GPIO方向
    char gpioDirPath[40];
    sprintf(gpioDirPath, "/sys/class/gpio/gpio%d/direction", this->ledNo);
    fd = open(gpioDirPath, O_WRONLY);
    if (fd < 0)
    {
        printf("gpioDirPath:%s\topen failed", gpioDirPath);
        exit(1);
    }
    sprintf(buf, "high");
    write(fd, buf, 4);
    close(fd);
}

// 设置LED状态 false亮， true灭
void LEDControl::setLedStatus(bool status)
{
    this->ledStatus = status;
    updateLedStatus();
    qDebug() << "led" << this->ledNo << "\t status:" << this->ledStatus;
}

// 更新LED状态
void LEDControl::updateLedStatus()
{
    int fd;
    char buf[2];
    char path[30];
    sprintf(path, "/sys/class/gpio/gpio%d/value", this->ledNo);

    fd = open(path, O_WRONLY);
    if (fd < 0)
    {
        printf("updateLedStatus failed");
        exit(1);
    }

    if (this->ledStatus)
    {
        buf[0] = '1';
    }
    else
    {
        buf[0] = '0';
    }
    write(fd, buf, 1);

    close(fd);
}
