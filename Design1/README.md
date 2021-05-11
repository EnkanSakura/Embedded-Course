# Design 1: 自动进样器控制系统

## 文件注释

 `main.c` 单片机程序源文件

 `gui.py` GUI程序源文件

 `form.py` UI类源文件（由pyuic5生成）

 `form.ui` QtUI设计文件

## 使用库

 `sys` 接收程序参数，用于创建QtApp实例

 `serial` 串口库，用于进行串口通信

 `PyQt` QtGUI库，用于实现GUI界面

## 函数注释

### `main.c`

#### 中断处理函数

 `void CONTROL_IRQ() interrupt 0` 外部中断0，用于接收并解码遥控器信号

 `void TIMER0_IRQ() interrupt 1` 定时器中断0，用于产生电机脉冲

 `void UART_IRQ() interrupt 4` UART中断，用于接收并解析串口数据

#### 其他功能函数

 `void Init_Int0()` 外部中断0参数设置

 `void Init_Timer0()` 定时器中断0参数设置

 `void Init_UART(unsigned int)` UART中断参数设置，参数为串口通信波特率

 `void UART_Solve()` UART接收数据处理

 `void Set_Motor()` 电机工作参数设置，用于控制电机工作

 `void Turn_Initial()` 转盘位置初始化

 `void Send_Char(char)` 串口单字符发送

 `void delay(unsigned int)` 延迟函数（片内两个定时器均被占用，延迟通过专门函数产生）

 `void keys(unsigned char)` 遥控器信号处理

### `gui.py`

#### 主要功能函数

 `get_bit(data, index)` 获取十六进制数某一二进制位

 `Pyqt_Serial.init(self)` GUI控件连接，timer创建与连接

 `Pyqt_Serial.port_check(self)` 串口检测，存储于类成员变量中

 `Pyqt_Serial.port_info(self)` 串口信息显示到组合框中

 `Pyqt_Serial.port_open(self)` 打开组合框选择的串口

 `Pyqt_Serial.port_close(self)` 关闭当前打开的串口

 `Pyqt_Serial.data_send(self, data: str)` 串口发送数据

 `Pyqt_Serial.data_receive(self)` 串口接收数据及解析

 `Pyqt_Serial.data_solve(self)` 接收信号处理，更改类成员变量

 `Pyqt_Serial.data_send_timer(self)` 定时询问下位机状态

 `Pyqt_Serial.data_update(self)` GUI界面数据更新

#### 指令封装函数

 `Pyqt_Serial.turn_init_pos(self)` 转盘初始化位置矫正量更改

 `Pyqt_Serial.turn_init(self)` 转盘初始化

 `Pyqt_Serial.turn_next(self)` 转盘移动到下一个位置

 `Pyqt_Serial.turn_forward(self)` 转盘移动到上一个位置

 `Pyqt_Serial.niddle_move(self, steps=None)` 针臂移动

 `Pyqt_Serial.statu_info(self)` 状态获取

 `Pyqt_Serial.staru_info_simple(self)` 位置状态获取

 `Pyqt_Serial.work_switch(self)` 工作开关