#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDateTime>
#include <QScrollBar>
#include <QTextCursor>
#include <QMessageBox>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , settings("UPB", "UPB UI")
{
    ui->setupUi(this);

    connect(ui->dsConnect, &QPushButton::clicked,
            this, &MainWindow::saveDevice);
    connect(ui->brsSave, &QPushButton::clicked,
            this, &MainWindow::saveBaudRate);
    connect(ui->dtsSend, &QPushButton::clicked,
            this, &MainWindow::saveDataBits);
    connect(ui->spsSave, &QPushButton::clicked,
            this, &MainWindow::saveStopBits);

    connect(ui->pbsEven, &QRadioButton::clicked,
            this, &MainWindow::saveParity);
    connect(ui->pbsOdd, &QRadioButton::clicked,
            this, &MainWindow::saveParity);
    connect(ui->pbsNone, &QRadioButton::clicked,
            this, &MainWindow::saveParity);

    connect(ui->cbTimestamp, &QCheckBox::toggled, this,
            [this](bool v){ settings.setValue(KEY_TIMESTAMP, v); });

    connect(ui->cbHexView, &QCheckBox::toggled, this,
            [this](bool v){ settings.setValue(KEY_HEX, v); });

    connect(&serial, &QSerialPort::readyRead,
            this, &MainWindow::onSerialDataReady);

    connect(&serial, &QSerialPort::errorOccurred,
            this, &MainWindow::onSerialError);

    connect(ui->smSend, &QPushButton::clicked,
            this, &MainWindow::onSendMessage);

    connect(this, &MainWindow::connectionStatusChanged,
            this, &MainWindow::updateConnectionStatus);

    heartbeatTimer = new QTimer(this);
    heartbeatTimer->setInterval(DEF_HEARTBEAT_INTERVAL);

    ensureDefaults();
    loadUiFromSettings();

    connect(heartbeatTimer, &QTimer::timeout,
            this, &MainWindow::checkConnectionHealth);
}

MainWindow::~MainWindow()
{
    serial.close();

    delete heartbeatTimer;
    delete ui;
}

void MainWindow::ensureDefaults()
{
    setConnectionState(ConnectionState::Disconnected);

    if (!settings.contains(KEY_DEVICE))
        settings.setValue(KEY_DEVICE, DEF_DEVICE);
    if (!settings.contains(KEY_BAUDRATE))
        settings.setValue(KEY_BAUDRATE, DEF_BAUDRATE);
    if (!settings.contains(KEY_DATABITS))
        settings.setValue(KEY_DATABITS, DEF_DATABITS);
    if (!settings.contains(KEY_STOPBITS))
        settings.setValue(KEY_STOPBITS, DEF_STOPBITS);
    if (!settings.contains(KEY_PARITY))
        settings.setValue(KEY_PARITY, PARITY_NONE);
    if (!settings.contains(KEY_LINE_END))
        settings.setValue(KEY_LINE_END, LINE_LF);

    if (!settings.contains(KEY_TIMESTAMP))
        settings.setValue(KEY_TIMESTAMP, true);
    if (!settings.contains(KEY_HEX))
        settings.setValue(KEY_HEX, false);
}

void MainWindow::loadUiFromSettings()
{
    ui->dsInput->setPlainText(settings.value(KEY_DEVICE).toString());

    ui->brsInput->setPlainText(
        QString::number(settings.value(KEY_BAUDRATE).toInt()));
    ui->dtsInput->setPlainText(
        QString::number(settings.value(KEY_DATABITS).toInt()));
    ui->spsInput->setPlainText(
        QString::number(settings.value(KEY_STOPBITS).toInt()));

    const QString parity = settings.value(KEY_PARITY).toString();
    ui->pbsEven->setChecked(parity == PARITY_EVEN);
    ui->pbsOdd->setChecked(parity == PARITY_ODD);
    ui->pbsNone->setChecked(parity == PARITY_NONE);

    ui->cbTimestamp->setChecked(settings.value(KEY_TIMESTAMP).toBool());
    ui->cbHexView->setChecked(settings.value(KEY_HEX).toBool());
    ui->cbLineEnding->setCurrentText(
        settings.value(KEY_LINE_END).toString());
}

void MainWindow::saveDevice()
{
    const QString dev = ui->dsInput->toPlainText().trimmed();
    settings.setValue(KEY_DEVICE, dev);
    reconnect();
}

void MainWindow::saveBaudRate()
{
    settings.setValue(KEY_BAUDRATE,
                      ui->brsInput->toPlainText().toInt());
    sendUartCfgFrame();
}

void MainWindow::saveDataBits()
{
    settings.setValue(KEY_DATABITS,
                      ui->dtsInput->toPlainText().toInt());
    sendUartCfgFrame();
}

void MainWindow::saveStopBits()
{
    settings.setValue(KEY_STOPBITS,
                      ui->spsInput->toPlainText().toInt());
    sendUartCfgFrame();
}

void MainWindow::saveParity()
{
    if (ui->pbsEven->isChecked())
        settings.setValue(KEY_PARITY, PARITY_EVEN);
    else if (ui->pbsOdd->isChecked())
        settings.setValue(KEY_PARITY, PARITY_ODD);
    else
        settings.setValue(KEY_PARITY, PARITY_NONE);
    sendUartCfgFrame();
}

void MainWindow::sendUartCfgFrame()
{
    uint32_t baud = uint32_t(settings.value(KEY_BAUDRATE).toInt());
    uint8_t dataBits = uint8_t(settings.value(KEY_DATABITS).toInt());
    uint8_t stopBits = uint8_t(settings.value(KEY_STOPBITS).toInt());

    const QString parityStr = settings.value(KEY_PARITY).toString();
    uint8_t parity = 0;
    if (parityStr == PARITY_EVEN)
        parity = 1;
    else if (parityStr == PARITY_ODD)
        parity = 2;

    QByteArray payload;
    payload.append(char(uint8_t(baud >> 24)));
    payload.append(char(uint8_t(baud >> 16)));
    payload.append(char(uint8_t(baud >> 8)));
    payload.append(char(uint8_t(baud)));
    payload.append(char(dataBits));
    payload.append(char(stopBits));
    payload.append(char(parity));

    sendDcpFrame(DcpCmd::SetCfg, DcpInterface::Uart, nextTxnId(), payload);
}

void MainWindow::reconnect()
{
    if (serial.isOpen())
        serial.close();

    setConnectionState(ConnectionState::Connecting);

    const QString dev = settings.value(KEY_DEVICE).toString();
    if (dev.isEmpty())
        return;

    serial.setPortName(dev);
    serial.open(QIODevice::ReadWrite);

    if (!serial.isOpen()) {
        setConnectionState(ConnectionState::Error);
        QMessageBox::warning(this, "Error", serial.errorString());
        return;
    }

    setConnectionState(ConnectionState::Connected);
}

void MainWindow::onSerialDataReady()
{
    lastRxTimer.restart();
    dcpParser.feed(serial.readAll());

    DcpFrame frame;
    DcpError err;
    DcpPopResult r;
    while ((r = dcpParser.popFrame(frame, err)) != DcpPopResult::None) {
        if (r == DcpPopResult::ProtocolError) {
            qWarning() << "DCP protocol error:" << dcpErrorToString(err);
            continue;
        }
        
        handleDcpFrame(frame);
    }
}

void MainWindow::handleDcpFrame(const DcpFrame &frame)
{
    if (frame.txnId == 0) {
        if (frame.cmd == DcpCmd::Data && frame.interface == DcpInterface::Uart)
            appendRx(frame.payload);

        return;
    }

    switch (frame.cmd) {
    case DcpCmd::Data:
        if (frame.interface == DcpInterface::Uart)
            appendRx(frame.payload);
        break;

    case DcpCmd::Err: {
        DcpError code = frame.payload.isEmpty()
                            ? DcpError::MalformedFrame
                            : static_cast<DcpError>(uint8_t(frame.payload.at(0)));
        appendRx(QByteArrayLiteral("ERR ") + dcpErrorToString(code).toUtf8());
        break;
    }

    default:
        break;
    }
}

void MainWindow::appendMessage(const QByteArray &data,
                               QLatin1StringView prefix,
                               const QColor &color)
{
    QTextCursor cursor(ui->messages->document());

    const bool atBottom =
        ui->messages->verticalScrollBar()->value() ==
        ui->messages->verticalScrollBar()->maximum();

    cursor.movePosition(QTextCursor::End);

    QTextCharFormat fmt;
    fmt.setForeground(color);

    QString text = settings.value(KEY_HEX).toBool()
                       ? data.toHex(' ')
                       : QString::fromUtf8(data);
    text = text.trimmed();

    if (settings.value(KEY_TIMESTAMP).toBool()) {
        cursor.insertText(QDateTime::currentDateTime()
                              .toString("[hh:mm:ss.zzz] "), fmt);
    }

    cursor.insertText(QStringLiteral("[") + prefix + "] " + text + "\n",
                      fmt);

    if (atBottom)
        ui->messages->verticalScrollBar()->setValue(
            ui->messages->verticalScrollBar()->maximum());
}

void MainWindow::appendRx(const QByteArray &data)
{
    appendMessage(data, QLatin1StringView{"RX"}, Qt::darkGreen);
}

void MainWindow::onSendMessage()
{
    if (!serial.isOpen()) {
        QMessageBox::warning(this, "UART",
                             "Serial port not open");
        return;
    }

    QByteArray data = ui->smInput->toPlainText().toUtf8();

    const QString lineEnding = ui->cbLineEnding->currentText();
    if (lineEnding == LINE_CRLF)
        data.append("\r\n");
    else
        data.append("\n");

    sendDcpFrame(DcpCmd::Run, DcpInterface::Uart, nextTxnId(), data);
    appendTx(data);
    ui->smInput->clear();
}

void MainWindow::appendTx(const QByteArray &data)
{
    appendMessage(data, QLatin1StringView{"TX"}, Qt::magenta);
}

uint8_t MainWindow::nextTxnId()
{
    do {
        txnIdCounter++;
    } while (txnIdCounter == 0);
    return txnIdCounter;
}

void MainWindow::sendDcpFrame(DcpCmd cmd, DcpInterface iface, uint8_t txnId,
                              const QByteArray &payload)
{
    if (!serial.isOpen()) return;
    serial.write(dcpEncode(cmd, iface, txnId, payload));
}

QSerialPort::Parity MainWindow::parityFromString(QStringView p) const
{
    if (p == PARITY_EVEN)
        return QSerialPort::EvenParity;
    if (p == PARITY_ODD)
        return QSerialPort::OddParity;
    return QSerialPort::NoParity;
}

void MainWindow::updateConnectionStatus(ConnectionState state)
{
    ui->adapterStatusLabel->setText(connectionStateText(state));
}

QString MainWindow::connectionStateText(ConnectionState state)
{
    switch (state) {
    case ConnectionState::Connected:
        return "🟢 Connected";
    case ConnectionState::Connecting:
        return "🟡 Connecting...";
    case ConnectionState::Disconnected:
        return "🔴 Disconnected";
    case ConnectionState::Error:
        return "❌ Error";
    default:
        return {};
    }
}

void MainWindow::setConnectionState(ConnectionState state)
{
    if (connectionState.has_value() && connectionState == state)
        return;

    connectionState = state;
    emit connectionStatusChanged(state);

    if (state == ConnectionState::Connected) {
        lastRxTimer.restart();
        heartbeatTimer->start();
    } else {
        heartbeatTimer->stop();
    }
}


void MainWindow::onSerialError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::NoError)
        return;

    qWarning() << serial.errorString();

    if (error == QSerialPort::ResourceError ||
        error == QSerialPort::DeviceNotFoundError ||
        error == QSerialPort::PermissionError)
    {
        serial.close();
        setConnectionState(ConnectionState::Disconnected);
    } else {
        setConnectionState(ConnectionState::Error);
    }
}

void MainWindow::checkConnectionHealth()
{
    if (connectionState != ConnectionState::Connected)
        return;

    if (lastRxTimer.elapsed() > MainWindow::HEARTBEAT_TIMEOUT) {
        qWarning() << "Connection lost (heartbeat timeout)";
        QMessageBox::warning(this, "Error", "Connection lost");
        serial.close();
        setConnectionState(ConnectionState::Disconnected);
    }
}
