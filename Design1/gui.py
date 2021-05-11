import sys
import serial
import serial.tools.list_ports
from PyQt5 import QtWidgets, QtGui
from PyQt5.QtWidgets import QMessageBox
from PyQt5.QtCore import QTimer, QThread
from form import Ui_Form


def get_bit(data, index):
    if data & (1 << index):
        return 1
    else:
        return 0


class Pyqt_Serial(QtWidgets.QWidget, Ui_Form):
    def __init__(self):
        super(Pyqt_Serial, self).__init__()
        self.setupUi(self)
        self.init()
        self.setWindowTitle('自动进样器控制台')
        self.ser = serial.Serial()
        self.port_check()

        self.rcv_list = [0]*10  # 数据接收缓存
        self.bottle_num = 50       # 总瓶数
        self.s3__edit_bottle_num.setText(str(self.bottle_num))
        self.cur_pos = -1       # 当前位置号
        self.s3__edit_cur_pos.setText(str(self.cur_pos))

        # 状态标记
        self.turn_motor_flag = False    # 转盘电机工作
        self.tturn_0_flag = False       # 位置为0
        self.turn_init_flag = False     # 转盘初始化
        self.niddle_motor_flag = False  # 针头电机工作
        self.niddle_limit_flag = False  # 针头限位
        self.is_work = False            # 工作
        # self.work_step = 0              # 工作状态步，0针下，1针上，2转盘
        # self.work_init_flag = 0         # 工作初始化，，，

    def init(self):
        # 输入框内容限制
        self.s2__edit_init_pos.setValidator(QtGui.QIntValidator())
        self.s2__edit_niddle_move.setValidator(QtGui.QIntValidator())

        # 串口检测按钮
        self.s1__btn_port_check.clicked.connect(self.port_check)
        # 串口信息显示
        self.s1__box_port_select.currentTextChanged.connect(self.port_info)
        # 打开串口按钮
        self.s1__btn_port_open.clicked.connect(self.port_open)
        # 关闭串口按钮
        self.s1__btn_port_close.clicked.connect(self.port_close)

        # 转盘初始化按钮
        self.s2__btn_turn_init.clicked.connect(self.turn_init)
        # 初始位置纠正按钮
        self.s2__btn_init_pos.clicked.connect(self.turn_init_pos)
        # 上一个位置按钮
        self.s2__btn_forward.clicked.connect(self.turn_forwart)
        # 下一个位置按钮
        self.s2__btn_next.clicked.connect(self.turn_next)
        # 针臂移动按钮
        self.s2__btn_niddle_move.clicked.connect(
            lambda: self.niddle_move(None))
        # 开始工作按钮
        self.s2__btn_work.clicked.connect(self.work_switch)

        # 定时询问状态
        self.timer_send = QTimer()
        self.timer_send.timeout.connect(self.statu_info)
        self.s3__edit_cur_pos.textChanged.connect(self.data_send_timer)
        # 定时接收数据
        self.timer_receive = QTimer()
        self.timer_receive.timeout.connect(self.data_receive)
        # 定时更新界面数据
        self.timer_update = QTimer()
        self.timer_update.timeout.connect(self.data_update)
        # 定时发送工作指令
        # self.timer_work = QTimer()
        # self.timer_work.timeout.connect(self.work)

        # 工作初始化定时器，，，
        # self.timer_work_init = QTimer()
        # self.timer_work_init.timeout.connect(self.work_init)

    # 串口检测
    def port_check(self):
        self.com_dict = {}
        port_list = list(serial.tools.list_ports.comports())
        self.s1__box_port_select.clear()
        for port in port_list:
            self.com_dict['%s' % port[0]] = '%s' % port[1]
            self.s1__box_port_select.addItem(port[0])
        if len(self.com_dict) == 0:
            self.s1__lb_state.setText('无可用串口')

    # 串口信息
    def port_info(self):
        info_s = self.s1__box_port_select.currentText()
        if info_s != '':
            self.s1__lb_state.setText(
                self.com_dict[self.s1__box_port_select.currentText()])

    # 打开串口
    def port_open(self):
        self.ser.port = self.s1__box_port_select.currentText()
        self.ser.baudrate = int(self.s1__box_baud_rate.currentText())
        self.ser.bytesize = 8
        self.ser.stopbits = 1
        self.ser.parity = 'N'

        try:
            self.ser.open()
        except:
            QMessageBox.critical(self, 'Port Error', '串口无法打开！')
            return None

        self.timer_send.start(1000)
        self.timer_receive.start(500)
        self.timer_update.start(100)
        # self.timer_work.start(3000)

        if self.ser.isOpen():
            self.s1__btn_port_open.setEnabled(False)
            self.s1__btn_port_close.setEnabled(True)
            self.s1__lb_state.setText(
                '串口 %s 已开启' % self.s1__box_port_select.currentText())

    # 关闭串口
    def port_close(self):
        self.timer_send.stop()
        self.timer_receive.stop()
        self.timer_update.stop()
        # self.timer_work.stop()

        try:
            self.ser.close()
        except:
            pass
        self.s1__btn_port_open.setEnabled(True)
        self.s1__btn_port_close.setEnabled(False)
        self.s1__lb_state.setText('串口已关闭')

    # 发送数据
    def data_send(self, data: str):
        if self.ser.isOpen():
            num = self.ser.write(bytes.fromhex(data))
            return num
        else:
            return None

    # 接收数据
    def data_receive(self):
        try:
            num = self.ser.inWaiting()
        except:
            self.port_close()
            return None
        rcv_num = 0
        rcv_step = 0
        # print('receive')
        if num > 0:
            for i in range(0, num):
                data = self.ser.read(1)[0]
                # data = binascii.b2a_hex(data)
                # print('{:02x}'.format(data))
                if data == 0x02:
                    self.rcv_list[0] = data
                    rcv_num = 1
                    rcv_step = 1
                elif rcv_step == 0:
                    self.rcv_list[0] = data
                    if self.rcv_list == 0x02:
                        rcv_num = 1
                        rcv_step = 1
                elif rcv_step == 1:
                    # print('s1')
                    self.rcv_list[1] = data
                    rcv_num = 2
                    rcv_step = 2
                elif rcv_step == 2:
                    # print('s2')
                    self.rcv_list[2] = data
                    rcv_num = 3
                    rcv_step = 3
                elif rcv_step == 3:
                    # print('s3')
                    self.rcv_list[3] = data
                    if self.rcv_list[3] == 0x10:
                        rcv_step = 4
                    else:
                        rcv_num = 4
                        rcv_step = 5
                elif rcv_step == 4:
                    # print('s4')
                    self.rcv_list[3] = data - 0x40
                    rcv_num = 4
                    rcv_step = 5
                elif rcv_step == 5:
                    # print('s5')
                    self.rcv_list[4] = data
                    if self.rcv_list[4] == 0x10:
                        rcv_step = 6
                    else:
                        rcv_num = 5
                        rcv_step = 7
                elif rcv_step == 6:
                    # print('s6')
                    self.rcv_list[4] = data - 0x40
                    rcv_num = 5
                    rcv_step = 7
                elif rcv_step == 7:
                    # print('s7')
                    self.rcv_list[5] = data
                    if self.rcv_list[5] == 0x10:
                        rcv_step = 8
                    else:
                        rcv_num = 6
                        rcv_step = 9
                elif rcv_step == 8:
                    # print('s8')
                    self.rcv_list[5] = data - 0x40
                    rcv_num = 6
                    rcv_step = 9
                elif rcv_step == 9:
                    # print('s9')
                    self.rcv_list[6] = data
                    if self.rcv_list[6] == 0x03:
                        rcv_num = 7
                        rcv_step = 0
                    # print(self.rcv_list)
                    self.data_solve()

    # 接收信号处理
    def data_solve(self):
        data = list(self.rcv_list)
        if data[0] == 0x02 and data[6] == 0x03:
            if data[1] == 0x89:
                if data[2] == 0x14:     # 初始位置纠正
                    pass
                elif data[2] == 0x13:   # 回到初始位置
                    self.cur_pos = 0
                    pass
                elif data[2] == 0x15:   # 转盘下一个位置
                    # self.cur_pos += (data[3] >= 0x80)
                    pass
                elif data[2] == 0x22:
                    pass
            elif data[1] == 0x81:
                if data[2] == 0x01:
                    bp = data[3]
                    ms = data[4]
                    pos = 0

                    # 主转盘到达1号瓶
                    if get_bit(bp, 7) == 1:
                        self.tturn_0_flag = True
                        # print('1', True)
                    else:
                        self.tturn_0_flag = False
                        # print('1', False)
                    # 针头电机限位
                    if get_bit(bp, 6) == 1:
                        self.niddle_limit_flag = True
                        # print('2', True)
                    else:
                        self.niddle_limit_flag = False
                        # print('2', False)
                    # 位置号
                    for i in range(0, 6):
                        if i > 0:
                            pos = pos << 1
                        pos += get_bit(bp, 5-i)
                    self.cur_pos = pos

                    # 转盘电机状态
                    if get_bit(ms, 0) == 1:
                        self.turn_motor_flag = True
                        # print('3', True)
                    else:
                        self.turn_motor_flag = False
                        # print('3', False)
                    # 针头电机状态
                    if get_bit(ms, 1) == 1:
                        self.niddle_motor_flag = True
                        # print('4', True)
                    else:
                        self.niddle_motor_flag = False
                        # print('4', False)
                    # 转盘初始化状态
                    if get_bit(ms, 3) == 1:
                        self.turn_init_flag = True
                        # print('5', True)
                    else:
                        self.turn_init_flag = False
                        # print('5', False)

                elif data[2] == 0x15:
                    self.bottle_num = data[3]
                    self.cur_pos = data[4]

    # 定时询问状态
    def data_send_timer(self):
        if int(self.s3__edit_cur_pos.displayText()) == -1:
            self.timer_send.stop()
        else:
            self.timer_send.start(1000)

    # 界面数据更新
    def data_update(self):
        self.s3__box_turn_motor.setChecked(self.turn_motor_flag)
        self.s3__box_niddle_motor.setChecked(self.niddle_motor_flag)
        self.s3__box_niddle_limit.setChecked(self.niddle_limit_flag)
        self.s3__box_turn_0.setChecked(self.tturn_0_flag)
        self.s3__box_turn_init.setChecked(self.turn_init_flag)
        self.s3__edit_cur_pos.setText(str(self.cur_pos))
        self.s3__edit_bottle_num.setText(str(self.bottle_num))

    # 发送指令封装
    def turn_init_pos(self):
        return self.data_send('02 09 14 00 {:02x} 00 03'.format(
            int(self.s2__edit_init_pos.displayText())
        ))

    def turn_init(self):
        self.turn_init_flag = True
        return self.data_send('02 09 13 00 00 00 03')

    def turn_next(self):
        self.turn_motor_flag = True
        return self.data_send('02 09 15 00 00 00 03')

    def turn_forward(self):
        self.turn_motor_flag = True
        return self.data_send('02 09 15 80 00 00 03')

    def niddle_move(self, steps=None):
        if not steps:
            try:
                steps = int(self.s2__edit_niddle_move.displayText())
            except:
                steps = 0
        print(steps)
        if steps == 0:
            return 0
        elif steps < 0:
            steps = abs(steps)
            steps = steps | 0x8000
        print('{:02x} {:02x}'.format((steps & 0xff00) >> 8, steps & 0xff))
        self.niddle_motor_flag = True
        return self.data_send('02 09 22 {:02x} {:02x} 00 03'.format(
            (steps & 0xff00) >> 8,
            steps & 0xff
        ))

    def statu_info(self):
        return self.data_send('02 01 01 00 08 00 03')

    def statu_info_simple(self):
        return self.data_send('02 01 15 00 08 00 03')

    def work_switch(self):
        if self.is_work:
            self.s2__btn_work.setText('开始工作')
            self.s2__rad_work.setChecked(False)
            self.is_work = False
            return self.data_send('02 09 12 80 00 00 03')
        else:
            self.s2__btn_work.setText('停止工作')
            self.s2__rad_work.setChecked(True)
            self.is_work = True
            return self.data_send('02 09 12 00 00 00 03')
            # self.timer_work_init.start(500)

    # # 工作初始化，，，
    # def work_init(self):
    #     self.niddle_move(1)
    #     self.work_init_flag += 1
    #     if self.work_init_flag > 5:
    #         self.timer_work_init.stop()

    # 工作
    # def work(self):
        # print('work f')
        # print('1', self.is_work, self.work_init_flag)
        # print(self.turn_motor_flag, self.niddle_motor_flag, self.turn_init_flag)

        # if self.turn_motor_flag or self.niddle_motor_flag or self.turn_init_flag:
        #     pass
        # elif self.is_work and (self.work_init_flag > 5):
        #     # print('2', self.is_work, self.work_init_flag)
        #     # print(self.work_step)
        #     if self.work_step == 0:
        #         # print('n d')
        #         self.niddle_move(600)
        #         if self.niddle_limit_flag:
        #             self.work_step = 1
        #     elif self.work_step == 1:
        #         # print('n u')
        #         self.niddle_move(-600)
        #         if self.niddle_limit_flag:
        #             self.work_step = 2
        #     elif self.work_step == 2:
        #         # print('t')
        #         self.turn_next()
        #         self.work_step = 0

        # return self.data_send('02 09 12 00 00 00 03')


if __name__ == '__main__':
    app = QtWidgets.QApplication(sys.argv)
    mw = Pyqt_Serial()
    mw.show()
    sys.exit(app.exec_())
