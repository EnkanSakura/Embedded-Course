#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include "ledcontrol.h"
#include "adreader.h"
#include "buzzer.h"
#include "qcustomplot.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>

namespace Ui
{
    class MainWidget;
}

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = 0);
    ~MainWidget();
    // 设置折线属性
    void setupRealtimeDataDemo(QCustomPlot *customPlot);
    QString currentIP;
    quint16 currentPort;
    QDialog *a;

private slots:
    void initialTCP();                    // 初始化TCP服务端
    void newConnect();                    // 建立客户端连接
    void sendMessage(QString infomation); // 发送消息
    void readMessage();                   // 接收消息
    void lostConnect();                   // 断开连接
    void analyzeMessage(QString msg);     // 解析消息
    void realtimeDataSlot();              // 绘制曲线
    void on_btn_led1_clicked();
    void on_btn_led2_clicked();
    void on_radio_led1_clicked();
    void on_radio_led2_clicked();
    void on_radio_led3_clicked();
    void on_radio_led4_clicked();
    void on_btn_pwm_clicked();

    void on_DisplayCurConBtn_clicked();
    void on_DisplayInterfaceAck_clicked();

private:
    Ui::MainWidget *ui;
    LEDControl *led1;                 // LED1
    LEDControl *led2;                 // LED2
    ADReader *adReader;               // AD电压模拟量读取类
    Buzzer *buzzer;                   // 蜂鸣器控制类
    QTimer dataTimer;                 // 定时器，周期调用realtimeDataSlot()槽，实现动态数据添加到曲线
    int SendTimeInterval;             // 发送时间
    QTcpSocket *clientSocket;         // 当前客户端
    QTcpServer *tcpServer;            // 服务端
    QList<QTcpSocket *> *socket_list; // 客户端列表
    QList<QString> *client_list;      // 客户端[IP:Port]列表
    QString serverPort;               // 服务端监听端口
    QString read_message;             // 当前读取信息
    bool adSendSwitch;                // AD数据发送给客户端的开关
    bool warningSwitch;               // 蜂鸣器报警开关
    int testVol;                      // 测试报警功能，初始值为0，开启测试后值为4000
    int C_L_count = 0;                // 指定server端坐上脚显示Ip列表的那一个
};

#endif // MAINWIDGET_H
