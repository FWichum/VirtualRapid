#include "virtualserialportcontroller.h"
#include <qiodevice.h>

#include <QIODevice>

#include <iostream>
#include <QSerialPort>

SerialPortController::SerialPortController(QString serialConnection)
{

    m_status_remoteStatus   = 0;
    m_status_errorType      = 0;
    m_status_errorPresent   = 0;
    m_status_replaceCoil    = 0;
    m_status_coilPresent    = 1;
    m_status_ready          = 0;
    m_status_armed          = 0;
    m_status_standby        = 1;

    this->m_address = serialConnection;
}


//*************************************************************************************************************

void SerialPortController::run()
{
    // N.B. most of these settings are actually the default in PySerial, but just being careful.
    QSerialPort porto;
    porto.setPortName(this->m_address);

    bool ok = porto.open(QIODevice::ReadWrite);
    std::cout << "Oeffne Port: " << (int) ok << std::endl;
    std::cout << "------------------------------------" << std::endl;
    porto.setBaudRate(QSerialPort::Baud9600);
    porto.setDataBits(QSerialPort::Data8);
    porto.setStopBits(QSerialPort::OneStop);
    porto.setParity(QSerialPort::NoParity);
    porto.setFlowControl(QSerialPort::NoFlowControl);
    // Make sure the RTS pin is set to off
    porto.setRequestToSend(false);

    while (true) {
        // ############### EINGABE ####################
        porto.waitForReadyRead(-1);
        QByteArray message = porto.readAll();
        std::cout << "Empfangen: " << message.toStdString() << std::endl;

        // ############### VERARBEITUNG ####################
        QByteArray returnmessage;
        returnmessage.append(message.at(0));

        if (message.at(0) == 'Q') {
            m_status_remoteStatus = 1;
            returnmessage.append(parseStatus());
        } else if (message.at(0) == 'N' && message.at(1) == 'D') {
            returnmessage.append("D7.2.0");
        } else if (message.at(0) == 'E' && message.at(1) == 'A') {
            m_status_ready          = 0;
            m_status_armed          = 0;
            m_status_standby        = 1;
            returnmessage.append('@');
        } else if (message.at(0) == 'E' && message.at(1) == 'B') {
            m_status_armed          = 1;
            m_status_standby        = 0;
            returnmessage.append('@');
        }else {
            returnmessage.append('?');
        }


        // ############### AUSGABE ####################


        char crc = calcCRC(returnmessage);
        returnmessage = returnmessage + crc;

        porto.write(returnmessage);
//        porto.waitForBytesWritten(300);
        std::cout << "- Gesendet: " << returnmessage.toStdString() << std::endl;
    }

    // If we get here, it's time to shutdown the serial port controller
    std::cout << "------------------------------------" << std::endl;
    std::cout << "Schließe Port." << std::endl;
    porto.close();
}

//*************************************************************************************************************

char SerialPortController::parseStatus()
{
    int status = 0;
    status = (status << 1) |  m_status_remoteStatus;
    status = (status << 1) |  m_status_errorType;
    status = (status << 1) |  m_status_errorPresent;
    status = (status << 1) |  m_status_replaceCoil;
    status = (status << 1) |  m_status_coilPresent;
    status = (status << 1) |  m_status_ready;
    status = (status << 1) |  m_status_armed;
    status = (status << 1) |  m_status_standby;

    return (char) status;
}


//*************************************************************************************************************

void SerialPortController::updateSerialWriteQueue(sendInfo info)
{
    // This locker will lock the mutex until it is destroyed, i.e. when this function call goes out of scope
    QMutexLocker locker(&m_mutex);
    this->m_serialWriteQueue.push(info);
}


//*************************************************************************************************************

char SerialPortController::calcCRC(QByteArray command)
{
    // Convert command string to sum of ASCII/byte values
    int commandSum = 0;
    for (int i = 0 ; i< command.length() ; i++) {
        commandSum += command.at(i);
    }
    // Convert command sum to binary, then invert and return 8-bit character value
    return (char) (~commandSum & 0xff);
}
