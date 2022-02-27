#include "mainwidget.h"
#include "ui_mainwidget.h"
#include <QDebug>
#include <QApplication>
#include <QMessageBox>
#include <QSound>

// 主窗体
MainWidget::MainWidget(QWidget *parent) : QWidget(parent), ui(new Ui::MainWidget)
{
    ui->setupUi(this);

    miniWindow = new QDialog;
    miniWindow->resize(400, 300);
    Iplabel = new QLabel(miniWindow);
    Portlabel = new QLabel(miniWindow);
    IpLineEdit = new QLineEdit(miniWindow);
    PortLineEdit = new QLineEdit(miniWindow);
    LinkBtn = new QPushButton(miniWindow);
    ExitBtn = new QPushButton(miniWindow);

    Iplabel->move(40, 40);
    Portlabel->move(40, 80);

    IpLineEdit->move(130, 40);
    PortLineEdit->move(130, 80);

    LinkBtn->move(90, 150);
    ExitBtn->move(220, 150);

    Iplabel->setText("目的ip:");
    Portlabel->setText("目的端口:");
    IpLineEdit->setText("192.168.43.200");
    PortLineEdit->setText("23333");
    LinkBtn->setText("连接");
    ExitBtn->setText("退出");

    // initial TCP Server
    this->tcpClient = new QTcpSocket(this); // 实例化tcpClient

    this->tcpClient->abort(); // 取消原有连接
    connect(tcpClient, SIGNAL(readyRead()), this, SLOT(ReadData()));
    connect(tcpClient, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(ReadError(QAbstractSocket::SocketError)));

    ui->IptextBrowser->setText(IpLineEdit->text());
    ui->PorttextBrowser->setText(PortLineEdit->text());
    // 未连接时，功能按钮禁止使用
    ui->TcpConnectBtn->setEnabled(true);
    ui->btn_led1->setEnabled(false);
    ui->btn_led2->setEnabled(false);
    ui->radio_led1->setEnabled(false);
    ui->radio_led2->setEnabled(false);
    ui->radio_led3->setEnabled(false);
    ui->radio_led4->setEnabled(false);

    // 初始化LED按钮颜色
    ui->btn_led1->setStyleSheet("QPushButton{background:white}");
    ui->btn_led2->setStyleSheet("QPushButton{background:white}");
    ui->radio_led1->setChecked(true);
    this->led1 = 1;
    this->led2 = 1;

    adSwitch = true; // AD读取开关，默认开启

    // initial Plot
    initPlot(ui->plot);
    ui->plot->replot();
    miniWindow->setModal(true);
    miniWindow->show();

    connect(LinkBtn, SIGNAL(clicked()), this, SLOT(on_LinkBtn_clicked()));
    connect(ExitBtn, SIGNAL(clicked()), this, SLOT(on_ExitBtn_clicked()));

    if (miniWindow->exec() == QDialog::Accepted)
    {
        miniWindow->close();
        exit(0);
    }
}

MainWidget::~MainWidget()
{
    delete ui;
}

// QCustomPlot初始化
void MainWidget::initPlot(QCustomPlot *customPlot)
{
    customPlot->addGraph();
    customPlot->graph(0)->setPen(QPen(Qt::red));
    customPlot->graph(0)->setName("Vol");

    //横坐标
    customPlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    customPlot->xAxis->setDateTimeFormat("hh:mm:ss");
    customPlot->xAxis->setAutoTickStep(false);
    customPlot->xAxis->setTickStep(2);
    customPlot->axisRect()->setupFullAxesBox();
    customPlot->legend->setVisible(true); //右上角小图标
}

// 更新Plot
void MainWidget::updatePlot(int value)
{
    if (adSwitch)
    {
        // 横轴：key 时间 单位s
        double key = QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000.0;
        // 纵轴：value 电压模拟量

        ui->lcd_vol->setDigitCount(4);
        ui->lcd_vol->setMode(QLCDNumber::Dec);
        ui->lcd_vol->display(QString::number(value));

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
}

void MainWidget::analysisMessage(QString msg)
{
    // 解析命令
    QStringList cmd = msg.split("#");
    // qDebug() << "命令个数:" << cmd.length() - 1;
    for (int i = 1; i < cmd.length(); i++)
    {
        qDebug() << "CMD[" << i << "]=" << cmd[i];
        if (cmd[i][0] == 'B')
        {
            int data = cmd[i].right(5).toInt();
            updatePlot(data);
        }
        else if (cmd[i][0] == 'L')
        {
            int data = cmd[i].right(2).toInt();
            qDebug() << "LED控制命令:" << data;

            switch (data)
            {
            case 11: // 1
                this->led1 = 1;
                this->led2 = 1;
                ui->btn_led2->setStyleSheet("QPushButton{background:white}");
                ui->btn_led1->setStyleSheet("QPushButton{background:white}");
                ui->radio_led1->setChecked(true);
                ui->radio_led2->setChecked(false);
                ui->radio_led3->setChecked(false);
                ui->radio_led4->setChecked(false);
                break;
            case 10: // 2
                this->led2 = 1;
                this->led1 = 0;
                ui->btn_led2->setStyleSheet("QPushButton{background:white}");
                ui->btn_led1->setStyleSheet("QPushButton{background:green}");
                ui->radio_led1->setChecked(false);
                ui->radio_led2->setChecked(true);
                ui->radio_led3->setChecked(false);
                ui->radio_led4->setChecked(false);
                break;
            case 01: // 3
                this->led2 = 0;
                this->led1 = 1;
                ui->btn_led2->setStyleSheet("QPushButton{background:green}");
                ui->btn_led1->setStyleSheet("QPushButton{background:white}");
                ui->radio_led1->setChecked(false);
                ui->radio_led2->setChecked(false);
                ui->radio_led3->setChecked(true);
                ui->radio_led4->setChecked(false);
                break;
            case 00: // 4
                this->led1 = 0;
                this->led2 = 0;
                ui->btn_led2->setStyleSheet("QPushButton{background:green}");
                ui->btn_led1->setStyleSheet("QPushButton{background:green}");
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
                this->adSwitch = true;
                break;
            case 0:
                this->adSwitch = false;
                break;
            default:
                this->adSwitch = true;
                qDebug() << "AD sender error";
            }
        }
        else if (cmd[i][0] == 'W')
        {
            int data = cmd[i].right(1).toInt();
            qDebug() << "报警提示命令:" << data;

            switch (data)
            {
            case 1:
                QSound::play(":/media/warning.wav");
                break;
            default:
                qDebug() << "Alert error";
            }
        }
    }
}

void MainWidget::ReadData()
{
    QString message = tcpClient->readAll();
    if (!message.isEmpty())
    {
        analysisMessage(message);
    }
}

void MainWidget::ReadError(QAbstractSocket::SocketError)
{
    tcpClient->disconnectFromHost();
    ui->TcpConnectBtn->setText(tr("连接"));
    QMessageBox msgBox;
    msgBox.setText(tr("failed to connect server because %1").arg(tcpClient->errorString()));
    msgBox.exec();
}

// LED1按钮点击事件
void MainWidget::on_btn_led1_clicked()
{
    if (led1)
    {
        QString buf = QString("#L%1%2").arg(this->led2).arg(0);
        qDebug() << buf;
        this->tcpClient->write(buf.toLatin1());
        ui->btn_led1->setStyleSheet("QPushButton{background:green}");
        this->led1 = 0;
    }
    else
    {
        QString buf = QString("#L%1%2").arg(this->led2).arg(1);
        qDebug() << buf;
        this->tcpClient->write(buf.toLatin1());
        ui->btn_led1->setStyleSheet("QPushButton{background:white}");
        this->led1 = 1;
    }
}

// LED2按钮点击事件
void MainWidget::on_btn_led2_clicked()
{
    if (led2)
    {
        QString buf = QString("#L%1%2").arg(0).arg(this->led1);
        qDebug() << buf;
        this->tcpClient->write(buf.toLatin1());
        ui->btn_led2->setStyleSheet("QPushButton{background:green}");
        this->led2 = 0;
    }
    else
    {
        QString buf = QString("#L%1%2").arg(1).arg(this->led1);
        qDebug() << buf;
        this->tcpClient->write(buf.toLatin1());
        ui->btn_led2->setStyleSheet("QPushButton{background:white}");
        this->led2 = 1;
    }
}

// LEDraido点击事件
void MainWidget::on_radio_led1_clicked()
{
    QString buf = QString("#L11");
    qDebug() << buf;
    this->tcpClient->write(buf.toLatin1());
    this->led1 = 1;
    this->led2 = 1;
}
void MainWidget::on_radio_led2_clicked()
{
    QString buf = QString("#L10").arg(0).arg(this->led1);
    qDebug() << buf;
    this->tcpClient->write(buf.toLatin1());
    this->led1 = 1;
    this->led2 = 0;
}
void MainWidget::on_radio_led3_clicked()
{
    QString buf = QString("#L01").arg(0).arg(this->led1);
    qDebug() << buf;
    this->tcpClient->write(buf.toLatin1());
    this->led1 = 0;
    this->led2 = 1;
}
void MainWidget::on_radio_led4_clicked()
{
    QString buf = QString("#L00").arg(0).arg(this->led1);
    qDebug() << buf;
    this->tcpClient->write(buf.toLatin1());
    this->led1 = 0;
    this->led2 = 0;
}

// 退出按钮点击事件
void MainWidget::on_btn_quit_clicked()
{
    qApp->quit();
    miniWindow->accept();
    qApp->exit(0);
}
void MainWidget::on_ExitBtn_clicked()
{
    miniWindow->accept();
}

// 连接按钮点击事件
void MainWidget::on_LinkBtn_clicked()
{
    tcpClient->connectToHost(IpLineEdit->text(), PortLineEdit->text().toInt());
    if (tcpClient->waitForConnected(1000))
    {
        ui->btn_led1->setEnabled(true);
        ui->btn_led2->setEnabled(true);
        ui->radio_led1->setEnabled(true);
        ui->radio_led2->setEnabled(true);
        ui->radio_led3->setEnabled(true);
        ui->radio_led4->setEnabled(true);
        ui->btn_adSwitch->setEnabled(true);
        miniWindow->close();
    }
    else
    {
        qDebug() << "有误";
    }
}
void MainWidget::on_TcpConnectBtn_clicked()
{
    if (ui->TcpConnectBtn->text() == "连接")
    {
        miniWindow->setModal(true);
        miniWindow->show();
        if (miniWindow->exec() == QDialog::Accepted)
        {
            miniWindow->close();
        }
        else
        {
            this->tcpClient->abort(); // 取消原有连接
            tcpClient->connectToHost(IpLineEdit->text(), PortLineEdit->text().toInt());

            if (tcpClient->waitForConnected(1000)) // 连接成功
            {
                ui->IptextBrowser->setText(IpLineEdit->text());
                ui->PorttextBrowser->setText(PortLineEdit->text());

                ui->TcpConnectBtn->setText("断开");
                ui->btn_led1->setEnabled(true);
                ui->btn_led2->setEnabled(true);
                ui->btn_adSwitch->setEnabled(true);
            }
        }
    }
    else
    {
        tcpClient->disconnectFromHost();
        if (tcpClient->state() == QAbstractSocket::UnconnectedState || tcpClient->waitForDisconnected(1000)) //已断开连接则进入if{}
        {
            ui->IptextBrowser->setText("");
            ui->PorttextBrowser->setText("");
            ui->btn_led1->setEnabled(false);
            ui->btn_led2->setEnabled(false);
            ui->btn_adSwitch->setEnabled(false);
            ui->TcpConnectBtn->setText("连接");
        }
    }
}
void MainWidget::on_btn_adSwitch_clicked()
{
    if (adSwitch)
    {
        adSwitch = false;
        ui->btn_adSwitch->setText("开启");
    }
    else
    {
        adSwitch = true;
        ui->btn_adSwitch->setText("关闭");
    }
}
