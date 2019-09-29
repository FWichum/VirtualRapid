#ifndef SERIALPORTCONTROLLER_H
#define SERIALPORTCONTROLLER_H

//*************************************************************************************************************
//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include <queue>

//*************************************************************************************************************
//=============================================================================================================
// Qt INCLUDES
//=============================================================================================================

#include <QtSerialPort/QSerialPort>
#include <QThread>
#include <QByteArray>
#include <QMutexLocker>
#include <QMutex>

//*************************************************************************************************************
//=============================================================================================================
// FORWARD DECLARATIONS
//=============================================================================================================

typedef std::tuple<QByteArray, QString, int> sendInfo;
typedef std::tuple<int, QByteArray> reciveInfo;
Q_DECLARE_METATYPE(sendInfo);
Q_DECLARE_METATYPE(reciveInfo);

//=============================================================================================================
/**
* The class creates a Thread which has direct control of the serial port. Commands for relaying via the
* serial port are received from signals.
*   N.B. Note that all functions except for run, are run in the callers thread
*
* @brief Controls the serial port.
*/

class SerialPortController : public QThread
{
    Q_OBJECT

public:
    //=========================================================================================================
    /**
    * Constructs a SerialPortController
    *
    * @param[in] serialConnection           The serial port
    * @param[in] serialWriteQueue           TODO Doxygen
    * @param[in] serialReadQueue            TODO Doxygen
    */
    SerialPortController(QString serialConnection);

    //=========================================================================================================
    /**
    * Continuously monitor the serialWriteQueue for commands from other Threads to be sent to the Magstim.
    * When requested, will return the automated reply from the Magstim unit to the calling process via signals.
    *   N.B. This should be called via start()
    */
    void run() override;

private:
    std::queue<std::tuple<QByteArray, QString, int>> m_serialWriteQueue;      /**< Queue for writing to Magstim */
    std::queue<std::tuple<int, QByteArray>> m_serialReadQueue;                /**< Queue for reading from Magstim */
    QString m_address;                                                        /**<  Adress of port */
    QMutex m_mutex;                                                           /**< To protect data in this thread */

    const int SERIAL_WRITE_ERROR = 1; // SERIAL_WRITE_ERR: Could not send the command.
    const int SERIAL_READ_ERROR  = 2; // SERIAL_READ_ERR:  Could not read the magstim response.

    int m_status_remoteStatus;
    int m_status_errorType;
    int m_status_errorPresent;
    int m_status_replaceCoil;
    int m_status_coilPresent;
    int m_status_ready;
    int m_status_armed;
    int m_status_standby;

    char parseStatus();
    char calcCRC(QByteArray command);
public slots:
    //=========================================================================================================
    /**
    * Updates the write queue to send something to Magstim.
    *
    * @param[in] info                       TODO Doxygen
    */
    void updateSerialWriteQueue(sendInfo info);

signals:
    //=========================================================================================================
    /**
    * A message from the Magstim unit was read.
    */
    void updateSerialReadQueue(const reciveInfo &info);
};

#endif // SERIALPORTCONTROLLER_H
