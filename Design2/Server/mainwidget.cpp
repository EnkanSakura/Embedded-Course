#include "mainwidget.h"
#include "ui_mainwidget.h"
#include <QDebug>
#include <QApplication>
#include <QDialog>

/**
 * @brief 主窗体
 * @param parent
 */
MainWidget::MainWidget(QWidget *parent) : QWidget(parent), ui(new Ui::MainWidget)
{
    ui->setupUi(this);
    ui->CurClientIptTextBrowser->setVisible(false);

    // initial LED
    this->led1 = new LEDControl(43); // GPIO43引脚控制LED1
    this->led2 = new LEDControl(44); // GPIO44引脚控制LED2

    ui->radio_led1->setEnabled(true);
    ui->radio_led2->setEnabled(true);
    ui->radio_led3->setEnabled(true);
    ui->radio_led4->setEnabled(true);

    // initial AD
    this->adReader = new ADReader(); // AD电压模拟量读取类

    //initial Buzzer
    this->buzzer = new Buzzer(); // 蜂鸣器控制类
    this->testVol = 0;

    // initial Plot
    SendTimeInterval = 500; // 发送时间间隔
    // 开始绘制
    setupRealtimeDataDemo(ui->plot);
    ui->plot->replot();

    // initial TCP Server
    this->serverPort = "23333"; // 默认监听端口23333
    ui->dis_cur_Port->setText(this->serverPort);

    initialTCP(); // 初始化Tcp

    adSendSwitch = true; // 默认开启 AD数据发送给客户端

    // 初始化LED按钮颜色
    ui->btn_led1->setStyleSheet("QPushButton{background:rgb(136, 138, 133)}");
    ui->btn_led2->setStyleSheet("QPushButton{background:rgb(136, 138, 133)}");
}

MainWidget::~MainWidget()
{
    this->buzzer->setEnable(0); // 关闭蜂鸣器
    this->tcpServer->close();   // TCPSocket关闭
    this->dataTimer.stop();     // 定时器关闭
    delete ui;
}

// 初始化TCP服务端
void MainWidget::initialTCP()
{
    tcpServer = new QTcpServer(this);
    clientSocket = new QTcpSocket(this);
    socket_list = new QList<QTcpSocket *>;
    client_list = new QList<QString>;
    //设置监听端口
    tcpServer->listen(QHostAddress::Any, this->serverPort.toInt());

    //新客户端连接
    connect(tcpServer, &QTcpServer::newConnection, this, &MainWidget::newConnect);
}

// 新客户端连接
void MainWidget::newConnect()
{
    clientSocket = tcpServer->nextPendingConnection();
    qDebug() << "新客户端连接:"
             << QHostAddress(clientSocket->peerAddress().toIPv4Address()).toString();
    //有可读消息
    connect(clientSocket, SIGNAL(readyRead()),this, SLOT(readMessage()));
    //断开连接
    connect(clientSocket, SIGNAL(disconnected()), this, SLOT(lostConnect()));

    //地址
    currentIP = QHostAddress(clientSocket->peerAddress().toIPv4Address()).toString();
    currentPort = clientSocket->peerPort();
    //添加到显示面板
    client_list->push_back(currentIP + ":" + QString::number(currentPort));
    //添加到列表
    socket_list->push_back(clientSocket);
}

// 发送消息
void MainWidget::sendMessage(QString infomation)
{
    QByteArray b = infomation.toLatin1();

    char *msg = b.data();
    for (int i = 0; i < socket_list->length(); i++)
    { //向每一个客户socket套接字传信息
        socket_list->at(i)->write(msg);
        qDebug() << "Send:" << msg;
    }
}

// @brief 接收消息
void MainWidget::readMessage()
{
    //遍历客户端列表，所有客户端
    for (int i = 0; i < socket_list->length(); i++) //读取每个客户端发来信息
    {

        read_message = socket_list->at(i)->readAll();
        if (!(read_message.isEmpty()))
        {
            qDebug() << "读取消息 [ClientIP:" << QHostAddress(socket_list->at(i)->peerAddress().toIPv4Address()).toString() << "]";
            qDebug() << "[ClientMsg]:" << read_message;
            //回传同样的消息
            sendMessage(read_message);
            //解析消息
            analyzeMessage(read_message);
            break;
        }
    }
}

// 断开连接
void MainWidget::lostConnect()
{
    QTcpSocket *client = qobject_cast<QTcpSocket *>(sender());
    currentIP = QHostAddress(client->peerAddress().toIPv4Address()).toString();
    currentPort = client->peerPort();
    QString ipAndPort = currentIP + ":" + QString::number(currentPort);
    qDebug() << "客户端断开连接:" << currentIP;
    client_list->removeAll(ipAndPort);
    socket_list->removeOne(client);
}

// 解析客户端发送的消息
void MainWidget::analyzeMessage(QString msg)
{
    //解析命令
    QStringList cmd = msg.split("#");
    // qDebug() << "命令个数:" << cmd.length() - 1;
    for (int i = 1; i < cmd.length(); i++)
    {
        // qDebug() << "CMD[" << i << "]=" << cmd[i];
        if (cmd[i][0] == 'L')
        {
            int data = cmd[i].right(2).toInt();
            qDebug() << "LED控制命令:" << data;

            switch (data)
            {
            case 11: // 1
                this->led1->setLedStatus(true);
                this->led2->setLedStatus(true);
                ui->btn_led2->setStyleSheet("QPushButton{background:rgb(136, 138, 133)}");
                ui->btn_led1->setStyleSheet("QPushButton{background:rgb(136, 138, 133)}");
                ui->radio_led1->setChecked(true);
                ui->radio_led2->setChecked(false);
                ui->radio_led3->setChecked(false);
                ui->radio_led4->setChecked(false);
                break;
            case 10: // 2
                this->led2->setLedStatus(true);
                this->led1->setLedStatus(false);
                ui->btn_led2->setStyleSheet("QPushButton{background:rgb(136, 138, 133)}");
                ui->btn_led1->setStyleSheet("QPushButton{background:red}");
                ui->radio_led1->setChecked(false);
                ui->radio_led2->setChecked(true);
                ui->radio_led3->setChecked(false);
                ui->radio_led4->setChecked(false);
                break;
            case 01: // 3
                this->led2->setLedStatus(false);
                this->led1->setLedStatus(true);
                ui->btn_led2->setStyleSheet("QPushButton{background:red}");
                ui->btn_led1->setStyleSheet("QPushButton{background:rgb(136, 138, 133)}");
                ui->radio_led1->setChecked(false);
                ui->radio_led2->setChecked(false);
                ui->radio_led3->setChecked(true);
                ui->radio_led4->setChecked(false);
                break;
            case 00: // 4
                this->led1->setLedStatus(false);
                this->led2->setLedStatus(false);
                ui->btn_led2->setStyleSheet("QPushButton{background:red}");
                ui->btn_led1->setStyleSheet("QPushButton{background:red}");
                ui->radio_led1->setChecked(false);
                ui->radio_led2->setChecked(false);
                ui->radio_led3->setChecked(false);
                ui->radio_led4->setChecked(true);
                break;
            default:
                qDebug() << "LED control error";
            }
        }
        else if (cmd[i][0] == 'R')
        {
            int data = cmd[i].right(1).toInt();
            qDebug() << "读取AD命令:" << data;

            switch (data)
            {
            case 1:
                this->adSendSwitch = true;
                break;
            case 0:
                this->adSendSwitch = false;
                break;
            default:
                this->adSendSwitch = true;
                qDebug() << "AD reader error";
            }
        }
        else if (cmd[i][0] == 'W')
        {
            int data = cmd[i].right(1).toInt();
            qDebug() << "报警提示命令:" << data;

            switch (data)
            {
            case 1:
                this->warningSwitch = true;
                break;
            case 0:
                this->warningSwitch = false;
                break;
            default:
                qDebug() << "Alert error";
            }
        }
    }
}

// QCustomPlot初始化
void MainWidget::setupRealtimeDataDemo(QCustomPlot *customPlot)
{
    customPlot->addGraph();
    customPlot->graph(0)->setPen(QPen(Qt::red));
    customPlot->graph(0)->setName("Vol");

    customPlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    customPlot->xAxis->setDateTimeFormat("hh:mm:ss");
    customPlot->xAxis->setAutoTickStep(false);
    // 绘制折线
    customPlot->xAxis->setTickStep(2);
    customPlot->axisRect()->setupFullAxesBox();

    // 通过QTimer定时获取数据并刷新Plot
    connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
    dataTimer.start(SendTimeInterval);
    customPlot->legend->setVisible(true); // 右上角小图标
}

// 绘制折线
void MainWidget::realtimeDataSlot()
{
    // 横轴：key 时间 单位s
    double key = QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000.0;
    // 纵轴：value 电压模拟量
    //int value = this->adReader->readVol(); // 读取AD
    int value = this->adReader->readVol() + this->testVol;

    // 检测电压范围，如超出0~4000，则开启蜂鸣器，报警
    if (value < 0 || value > 4000)
    {
        buzzer->setEnable(1); // 报警状态开启
        QString msg = QString("#W1");
        sendMessage(msg); // 远程通知客户端
    }
    else
    {
        buzzer->setEnable(0);
    }

    // 更新LCD液晶
    ui->lcd_vol->setDigitCount(4);
    ui->lcd_vol->setMode(QLCDNumber::Dec);
    ui->lcd_vol->display(QString::number(value));

    // 发送给客户端
    if (adSendSwitch)
    {
        QString msg = QString("#B%1").arg(value, 5, 10, QLatin1Char('0'));
        sendMessage(msg);
    }

    // 添加数据到曲线0
    ui->plot->graph(0)->addData(key, value);

    // 删除8秒之前的数据
    ui->plot->graph(0)->removeDataBefore(key - 8);

    // 自动设定graph曲线y轴的范围，如果不设定，有可能看不到图像
    ui->plot->yAxis->setRange(500, 7000);

    // 横坐标时间宽度为8秒，如果想要横坐标显示更多的时间，就把8调整为比较大到值
    // 这时removeDataBefore(key-8)中的8也要改成60，否则曲线显示不完整。
    ui->plot->xAxis->setRange(key + 0.25, 8, Qt::AlignRight); // 设定x轴的范围
    ui->plot->replot();
}

// LED1按钮点击事件
void MainWidget::on_btn_led1_clicked()
{
    if (led1->ledStatus)
    {
        led1->setLedStatus(false);
        ui->btn_led1->setStyleSheet("QPushButton{background:red}");
        if (led2->ledStatus)
        {
            ui->radio_led1->setChecked(false);
            ui->radio_led2->setChecked(false);
            ui->radio_led3->setChecked(true);
            ui->radio_led4->setChecked(false);
        }
        else
        {
            ui->radio_led1->setChecked(false);
            ui->radio_led2->setChecked(false);
            ui->radio_led3->setChecked(false);
            ui->radio_led4->setChecked(true);
        }
    }
    else
    {
        led1->setLedStatus(true);
        ui->btn_led1->setStyleSheet("QPushButton{background:rgb(136, 138, 133)}");
        if (led2->ledStatus)
        {
            ui->radio_led1->setChecked(true);
            ui->radio_led2->setChecked(false);
            ui->radio_led3->setChecked(false);
            ui->radio_led4->setChecked(false);
        }
        else
        {
            ui->radio_led1->setChecked(false);
            ui->radio_led2->setChecked(true);
            ui->radio_led3->setChecked(false);
            ui->radio_led4->setChecked(false);
        }
    }
    // 通知客户端
    QString msg = QString("#L%1%2").arg((int)led2->ledStatus).arg((int)led1->ledStatus);
    sendMessage(msg);
}

// LED2按钮点击事件
void MainWidget::on_btn_led2_clicked()
{
    if (led2->ledStatus)
    {
        led2->setLedStatus(false);
        ui->btn_led2->setStyleSheet("QPushButton{background:red}");
        if (led1->ledStatus)
        {
            ui->radio_led1->setChecked(false);
            ui->radio_led2->setChecked(true);
            ui->radio_led3->setChecked(false);
            ui->radio_led4->setChecked(false);
        }
        else
        {
            ui->radio_led1->setChecked(false);
            ui->radio_led2->setChecked(false);
            ui->radio_led3->setChecked(false);
            ui->radio_led4->setChecked(true);
        }
    }
    else
    {
        led2->setLedStatus(true);
        ui->btn_led2->setStyleSheet("QPushButton{background:rgb(136, 138, 133)}");
        if (led1->ledStatus)
        {
            ui->radio_led1->setChecked(true);
            ui->radio_led2->setChecked(false);
            ui->radio_led3->setChecked(false);
            ui->radio_led4->setChecked(false);
        }
        else
        {
            ui->radio_led1->setChecked(false);
            ui->radio_led2->setChecked(false);
            ui->radio_led3->setChecked(true);
            ui->radio_led4->setChecked(false);
        }
    }
    // 通知客户端
    QString msg = QString("#L%1%2").arg((int)led2->ledStatus).arg((int)led1->ledStatus);
    sendMessage(msg);
}

// LEDraido点击事件
void MainWidget::on_radio_led1_clicked()
{
    QString msg = QString("#L11");
    qDebug() << msg;
    this->led1->setLedStatus(true);
    this->led2->setLedStatus(true);
    ui->btn_led1->setStyleSheet("QPushButton{background:rgb(136, 138, 133)}");
    ui->btn_led2->setStyleSheet("QPushButton{background:rgb(136, 138, 133)}");
    sendMessage(msg);
}
void MainWidget::on_radio_led2_clicked()
{
    QString msg = QString("#L10");
    qDebug() << msg;
    this->led1->setLedStatus(true);
    this->led2->setLedStatus(false);
    ui->btn_led1->setStyleSheet("QPushButton{background:rgb(136, 138, 133)}");
    ui->btn_led2->setStyleSheet("QPushButton{background:red}");
    sendMessage(msg);
}
void MainWidget::on_radio_led3_clicked()
{
    QString msg = QString("#L01");
    qDebug() << msg;
    this->led1->setLedStatus(false);
    this->led2->setLedStatus(true);
    ui->btn_led1->setStyleSheet("QPushButton{background:red}");
    ui->btn_led2->setStyleSheet("QPushButton{background:rgb(136, 138, 133)}");
    sendMessage(msg);
}
void MainWidget::on_radio_led4_clicked()
{
    QString msg = QString("#L00");
    qDebug() << msg;
    this->led1->setLedStatus(false);
    this->led2->setLedStatus(false);
    ui->btn_led1->setStyleSheet("QPushButton{background:red}");
    ui->btn_led2->setStyleSheet("QPushButton{background:red}");
    sendMessage(msg);
}

// 测试报警功能，点击后AD读取电压+4000，模拟超出范围
void MainWidget::on_btn_pwm_clicked()
{
    if (this->testVol == 0)
    {
        this->testVol = 4000;
        ui->btn_pwm->setText("测试报警功能[开启]");
    }
    else
    {
        this->testVol = 0;
        ui->btn_pwm->setText("测试报警功能[关闭]");
    }
}

void MainWidget::on_DisplayCurConBtn_clicked()
{
    a = new QDialog;
    a->resize(400, 300);

    QTextBrowser *CurCon_TextBrower;
    CurCon_TextBrower = new QTextBrowser(a);
    QPushButton *ack;
    ack = new QPushButton(a);
    CurCon_TextBrower->resize(330, 150);
    CurCon_TextBrower->move(20, 10);

    ack->move(200, 200);
    ack->setText("确定");

    CurCon_TextBrower->clear();
    for (int i = 0; i < client_list->length(); i++)
    {
        CurCon_TextBrower->append(client_list->at(i));
    }
    a->setModal(true);
    a->show();
    connect(ack, SIGNAL(clicked()), this, SLOT(on_DisplayInterfaceAck_clicked()));
}

void MainWidget::on_DisplayInterfaceAck_clicked()
{
    a->close();
}
