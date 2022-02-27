#ifndef ADREADER_H
#define ADREADER_H

// AD读取类
class ADReader
{
public:
    ADReader();
    int readVol(); // 电压模拟量读取

private:
    char buf[20];   // AD读取缓冲区
    int readLen;    // 当次读取数据长度
    int adFileDesc; // AD设备文件描述符
};

#endif // ADREADER_H
