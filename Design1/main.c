#include <STC90C5xAD.H>

#define BOTTLE_NUM 50
#define ROUND_STEP 400
#define CRYSTAL_FREQUENCY 12000000
// #define CRYSTAL_FREQUENCY 11059200

// IO引脚接线定义
sbit Mp = P1 ^ 0;
sbit Md = P1 ^ 1;
sbit Fp = P1 ^ 2;
sbit Fd = P1 ^ 3;
sbit Pos = P0 ^ 0;
sbit Up = P0 ^ 1;
sbit Down = P0 ^ 2;
sbit Control = P3 ^ 2;

char PFlag = 0;
char tcount = 0;

char Motor = 0;
char MSpeed = 20;    // 电机转速
int MSteps = 0;      // 电机步数
char InitSteps = 22; // 初始化纠正步数

// 状态标记
char TurnFlag = 0;     // 转盘电机状态
char NiddleFlag = 0;   // 针头电机状态
int NiddleSteps = 0;   // 0无效，正数下降，负数上升
char TurnInitFlag = 0; // 转盘初始化状态
char NextFlag = -1;    // 转到下一个位置，-1无效，0顺时针，1逆时针
char CurBottle = 0;
char WorkFlag = 0;
char WorkStep = 0;

// 串口通信
unsigned char RcvBuf[10];  // 接收缓冲区
unsigned char RcvStep = 0; // 接收状态步标记
unsigned char RcvNum = 0;  // 接收字符数
unsigned char SndBuf[7];   // 发送缓冲区
char SendFlag = 0;         // 发送标记

// 遥控器
unsigned char IrValue[4];
unsigned char Time;
bit ControlFlag;

void Init_Int0();
void Init_Timer0();
void Init_UART(unsigned int baud);
void UART_Solve();
void Set_Motor();
void Turn_Initial();
void Send_Char(char);
void delay(unsigned int t);
void keys(unsigned char t);

void CONTROL_IRQ() interrupt 0
{
    unsigned char j, k;
    unsigned int err;
    Time = 0;
    delay(700);
    if (Control == 0)
    {
        err = 1000;
        while ((Control == 0) && (err > 0))
        {
            delay(1);
            err--;
        }
        if (Control == 1)
        {
            err = 500;
            while ((Control == 1) && (err > 0))
            {
                delay(1);
                err--;
            }
            for (k = 0; k < 4; k++)
            {
                for (j = 0; j < 8; j++)
                {
                    err = 60;
                    while ((Control == 0) && (err > 0))
                    {
                        delay(1);
                        err--;
                    }
                    err = 500;
                    while ((Control == 1) & (err > 0))
                    {
                        delay(10);
                        Time++;
                        err--;
                        if (Time > 30)
                        {
                            return;
                        }
                    }
                    IrValue[k] >>= 1;
                    if (Time >= 8)
                    {
                        IrValue[k] |= 0x80;
                    }
                    Time = 0;
                }
            }
        }
        if (IrValue[2] != ~IrValue[3])
        {
            ControlFlag = 1;
        }
        else
        {
            ControlFlag = 0;
        }
    }
}

void TIMER0_IRQ() interrupt 1
{
    TH0 = 0xfc;
    TL0 = 0x18;
    TR0 = 1;

    if (++tcount > MSpeed)
    {
        tcount = 0;
        if (MSteps)
        {
            MSteps--;
            PFlag = ~PFlag;
            if (Motor == 0)
            {
                Mp = PFlag;
            }
            else
            {
                Fp = PFlag;
                if ((Up == 0 && Fp == 0) || (Down == 0 && Fp == 1))
                {
                    MSteps = 0;
                }
            }
        }
    }
}

void UART_IRQ() interrupt 4
{
    int i;
    unsigned char c;
    if (RI)
    {
        c = SBUF;
        RI = 0;
        if (c == 0x02)
        {
            RcvBuf[0] = c;
            RcvNum = 1;
            RcvStep = 1;
        }
        else
        {
            switch (RcvStep)
            {
            case 0:
                RcvBuf[0] = c;
                if (RcvBuf[0] == 0x02)
                {
                    RcvNum = 1;
                    RcvStep = 1;
                }
                break;
            case 1:
                RcvBuf[1] = c;
                RcvNum = 2;
                RcvStep = 2;
                break;
            case 2:
                RcvBuf[2] = c;
                RcvNum = 3;
                RcvStep = 3;
                break;
            case 3:
                RcvBuf[3] = c;
                if (RcvBuf[3] == 0x10)
                {
                    RcvStep = 4;
                }
                else
                {
                    RcvNum = 4;
                    RcvStep = 5;
                }
                break;
            case 4:
                RcvBuf[3] = c - 0x40;
                RcvNum = 4;
                RcvStep = 5;
                break;
            case 5:
                RcvBuf[4] = c;
                if (RcvBuf[4] == 0x10)
                {
                    RcvStep = 6;
                }
                else
                {
                    RcvNum = 5;
                    RcvStep = 7;
                }
                break;
            case 6:
                RcvBuf[4] = c - 0x40;
                RcvNum = 5;
                RcvStep = 7;
                break;
            case 7:
                RcvBuf[5] = c;
                if (RcvBuf[5] == 0x10)
                {
                    RcvStep = 8;
                }
                else
                {
                    RcvNum = 6;
                    RcvStep = 9;
                }
                break;
            case 8:
                RcvBuf[5] = c - 0x40;
                RcvNum = 6;
                RcvStep = 9;
                break;
            case 9:
                RcvBuf[6] = c;
                if (RcvBuf[6] == 0x03)
                {
                    RcvNum = 7;
                    RcvStep = 0;
                    for (i = 0; i < 8; i++)
                    {
                        SndBuf[i] = RcvBuf[i];
                        RcvBuf[i] = 0x00;
                    }
                    UART_Solve();
                }
                break;
            default:
                break;
            }
        }
    }
    else
    {
        TI = 0;
    }
}

void delay(unsigned int t)
{
    while (t--)
        ;
}

void Init_Int0()
{
    IT0 = 1;
    EX0 = 1;
    EA = 1;
    Control = 1;
}

void Init_Timer0()
{
    TMOD |= 0x01;
    TH0 = 0xfc;
    TL0 = 0x18;
    // IE |= 0x82;
    EA = 1;
    ET0 = 1;
    TR0 = 1;
}

void Init_UART(unsigned int baud)
{
    SCON = 0x50;
    // TMOD |= 0x0f;
    TMOD |= 0x20;
    PS = 1;
    TH1 = 256 - (CRYSTAL_FREQUENCY / 12 / 32) / baud;
    TL1 = TH1;
    TR1 = 1;

    // IE |= 0x90;
    EA = 1;
    ES = 1;
}

void UART_Solve()
{
    if (SndBuf[0] == 0x02 && SndBuf[6] == 0x03)
    {
        if (SndBuf[1] == 0x09)
        {
            if (SndBuf[2] == 0x14) // 初始位置纠正
            {
                InitSteps = (SndBuf[4] >= 0x80 ? -1 : 1) * (SndBuf[4] & 0x7f);
                SendFlag = 1;
            }
            else if (SndBuf[2] == 0x13) // 转盘回到初始位置
            {
                TurnInitFlag = 1;
                SendFlag = 1;
            }
            else if (SndBuf[2] == 0x15) // 转盘转到下一个位置
            {
                NextFlag = SndBuf[3] >= 0x80;
                SendFlag = 1;
            }
            else if (SndBuf[2] == 0x22) // 针头升降
            {
                NiddleSteps = (SndBuf[3] >= 0x80 ? -1 : 1) * (((SndBuf[3] & 0x7f) << 8) + SndBuf[4]);
                NiddleFlag = 1;
                SendFlag = 1;
            }
            else if (SndBuf[2] == 0x12) // 工作开关
            {
                if (SndBuf[3] >= 0x80)
                {
                    NiddleSteps = -600;
                    NiddleFlag = 1;
                    WorkStep = 0;
                    WorkFlag = 0;
                    SendFlag = 1;
                }
                else
                {
                    WorkFlag = 1;
                    SendFlag = 1;
                }
            }
        }
        else if (SndBuf[1] == 0x01)
        {
            if (SndBuf[2] == 0x01) // 获取设备当前状态
            {
                SndBuf[3] = (((CurBottle == 0) << 7) + ((Up == 0) << 6) + CurBottle);
                SndBuf[4] = (((TurnFlag || NiddleFlag) << 7) + (TurnInitFlag << 3) + (NiddleFlag << 1) + TurnFlag);
                SendFlag = 1;
            }
            else if (SndBuf[2] == 0x15) // 获取总位置数和当前位置数
            {
                SndBuf[3] = BOTTLE_NUM;
                SndBuf[4] = CurBottle;
                SendFlag = 1;
            }
        }
        SndBuf[1] += 0x80;
    }
}

void Set_Motor(int motor, int steps)
{
    Motor = motor;
    MSteps = (steps < 0 ? -2 : 2) * steps;
    if (motor == 0)
    {
        Md = steps < 0;
    }
    else
    {
        Fd = steps > 0;
    }
}

void Turn_Initial()
{
    if (Up == 1)
    {
        Set_Motor(1, -400);
        while (Up == 1)
            ;
    }
    if (Pos == 0)
    {
        Set_Motor(0, -100);
        while (Pos == 0)
            ;
    }
    Set_Motor(0, 400);
    while (Pos == 1)
        ;
    Set_Motor(0, InitSteps);
    while (MSteps)
        ;
    CurBottle = 0;
}

void Send_Char(char c)
{
    SBUF = c;
    while (!TI)
        ;
    TI = 0;
}

void keys(unsigned char t)
{
    switch (t)
    {
    case 0x18: // 上
        Set_Motor(1, -600);
        break;
    case 0x52: // 下
        Set_Motor(1, 600);
        break;
    case 0x08: // 左
        Set_Motor(0, ROUND_STEP / BOTTLE_NUM);
        CurBottle++;
        if (CurBottle >= BOTTLE_NUM)
        {
            CurBottle = 0;
        }
        break;
    case 0x5A: // 右
        Set_Motor(0, -1 * ROUND_STEP / BOTTLE_NUM);
        CurBottle--;
        if (CurBottle < 0)
        {
            CurBottle = BOTTLE_NUM - 1;
        }
        break;
    case 0x1C: // OK
        TurnInitFlag = 1;
        break;
    default:
        break;
    }
}

main()
{
    int i;
    unsigned char key;
    TMOD = 0x00;
    IE = 0x00;
    SCON = 0x00;
    Init_Int0();
    Init_Timer0();
    Init_UART(1200);

    while (1)
    {
        if (!ControlFlag)
        {
            key = IrValue[2];
            if (key)
            {
                ControlFlag = 1;
            }
            keys(key);
        }
        if (SendFlag == 1)
        {
            // TI = 0;
            EA = 0;
            for (i = 0; i < 7; i++)
            {
                // SBUF = SndBuf[i];
                // while (!TI)
                //     ;
                // TI = 0;
                if (i >= 3 && i <= 5 && (SndBuf[i] == 0x02 || SndBuf[i] == 0x03 || SndBuf[i] == 0x10))
                {
                    Send_Char(0x10);
                    SndBuf[i] += 0x40;
                }
                Send_Char(SndBuf[i]);
                SndBuf[i] = 0x00;
            }
            SendFlag = 0;
            EA = 1;
        }
        if (MSteps == 0 && WorkFlag)
        {
            switch (WorkStep)
            {
            case 0:
                NiddleFlag = 1;
                NiddleSteps = 600;
                if (Down == 0)
                {
                    WorkStep = 1;
                }
                break;
            case 1:
                NiddleFlag = 1;
                NiddleSteps = -600;
                if (Up == 0)
                {
                    WorkStep = 2;
                }
                break;
            case 2:
                NextFlag = 0;
                if (MSteps == 0)
                {
                    WorkStep = 0;
                }
                break;
            default:
                break;
            }
        }
        if (TurnInitFlag)
        {
            Turn_Initial();
            TurnInitFlag = 0;
        }
        else if (NiddleFlag == 1)
        {
            Set_Motor(1, NiddleSteps);
            NiddleFlag = 0;
        }
        else if (NextFlag != -1)
        {
            Set_Motor(0, (ROUND_STEP / BOTTLE_NUM) * (NextFlag == 0 ? 1 : -1));
            if (CurBottle == 0 && NextFlag != 0)
            {
                CurBottle = BOTTLE_NUM - 1;
            }
            else if (CurBottle == BOTTLE_NUM - 1 && NextFlag == 0)
            {
                CurBottle = 0;
            }
            else
            {
                CurBottle += NextFlag == 0 ? 1 : -1;
            }
            NextFlag = -1;
        }
    }
}