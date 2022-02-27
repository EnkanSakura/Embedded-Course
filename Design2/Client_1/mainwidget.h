#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include "qcustomplot.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

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
    void initPlot(QCustomPlot *customPlot); // 设置折线属性
    void updatePlot(int value);             // 更新曲线
    void analysisMessage(QString msg);
    QDialog *miniWindow;
    QLabel *Iplabel;
    QLabel *Portlabel;
    QLineEdit *IpLineEdit;
    QLineEdit *PortLineEdit;
    QPushButton *LinkBtn;
    QPushButton *ExitBtn;

private slots:
    void ReadData();
    void ReadError(QAbstractSocket::SocketError);

    void on_btn_led1_clicked();
    void on_btn_led2_clicked();
    void on_radio_led1_clicked();
    void on_radio_led2_clicked();
    void on_radio_led3_clicked();
    void on_radio_led4_clicked();
    void on_btn_quit_clicked();
    void on_TcpConnectBtn_clicked();
    void on_btn_adSwitch_clicked();
    void on_LinkBtn_clicked();
    void on_ExitBtn_clicked();

private:
    Ui::MainWidget *ui;

    QTcpSocket *tcpClient;

    int led1;
    int led2;
    int adSwitch; // AD数据发送给客户端的开关
    bool AdSwitchState;
};

#endif // MAINWIDGET_H
