#ifndef BUZZER_H
#define BUZZER_H

// 蜂鸣器类
class Buzzer
{
public:
    Buzzer();
    void setEnable(int enable_);

private:
    void initPWM();
    void updateEnable();
    int enable; // 1开启，0关闭
};

#endif // BUZZER_H
