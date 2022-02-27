#include "buzzer.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <QDebug>

// 蜂鸣器
Buzzer::Buzzer()
{
    initPWM();
}

// 初始化PWM
void Buzzer::initPWM()
{
    int fd;
    char buf[15];
    char exportPath[35] = "/sys/class/pwm/pwmchip0/export";
    // 配置PWM
    fd = open(exportPath, O_WRONLY);
    if (fd < 0)
    {
        printf("pwmExportPath:%s\topen failed", exportPath);
        exit(1);
    }
    buf[0] = '0';
    write(fd, buf, 1);
    close(fd);

    // 设置PWM周期
    char periodPath[45] = "/sys/class/pwm/pwmchip0/pwm0/period";
    fd = open(periodPath, O_WRONLY);
    if (fd < 0)
    {
        printf("pwmPeriodPath:%s\topen failed", periodPath);
        exit(1);
    }
    sprintf(buf, "400000000");
    write(fd, buf, 9);
    close(fd);

    // 设置PWM占空比
    char dutyCyclePath[55] = "/sys/class/pwm/pwmchip0/pwm0/duty_cycle";
    fd = open(dutyCyclePath, O_WRONLY);
    if (fd < 0)
    {
        printf("pwmDutyCyclePath:%s\topen failed", dutyCyclePath);
        exit(1);
    }
    sprintf(buf, "100000000");
    write(fd, buf, 9);
    close(fd);
}

// 更新蜂鸣器状态
void Buzzer::updateEnable()
{
    int fd;
    char buf[2];
    char path[45] = "/sys/class/pwm/pwmchip0/pwm0/enable";
    fd = open(path, O_WRONLY);
    if (fd < 0)
    {
        printf("updatePWMEnable failed");
        exit(1);
    }

    buf[0] = '0' + this->enable;

    write(fd, buf, 1);

    close(fd);
}

// 设置蜂鸣器开关状态
void Buzzer::setEnable(int enable_)
{
    if (this->enable == enable_)
        return; // 防止重复写

    this->enable = enable_;
    updateEnable();
    qDebug() << "PWM: " << this->enable;
}
