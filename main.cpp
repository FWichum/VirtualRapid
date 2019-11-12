#include <iostream>


#include "virtualserialportcontroller.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc,argv);

    SerialPortController *port = new SerialPortController("COM20");
    port->start();



    return a.exec();
}
