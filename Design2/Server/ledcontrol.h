#ifndef LEDCONTROL_H
#define LEDCONTROL_H

// LED控制类
class LEDControl
{
public:
    LEDControl(int no);
    void initGPIO();                // 初始化GPIO引脚
    void setLedStatus(bool status); // 设置LED状态

private:
    void updateLedStatus(); // 更新GPIO引脚状态，控制LED亮灭

public:
    int ledNo;      // LED编号
    bool ledStatus; // LED状态
};

#endif // LEDCONTROL_H
