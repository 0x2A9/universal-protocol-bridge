#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSettings>
#include <QList>
#include <QStringView>
#include <QLatin1StringView>
#include <QElapsedTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum class ConnectionState {
        Disconnected,
        Connecting,
        Connected,
        Error
    };
    Q_ENUM(ConnectionState)

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void saveDevice();
    void saveBaudRate();
    void saveDataBits();
    void saveStopBits();
    void saveParity();

    void onSendMessage();
    void onSerialDataReady();
    void onSerialError(QSerialPort::SerialPortError error);
    void updateConnectionStatus(MainWindow::ConnectionState state);

private:
    inline static constexpr QLatin1StringView KEY_DEVICE    {"uart/device"};
    inline static constexpr QLatin1StringView KEY_BAUDRATE  {"uart/baudrate"};
    inline static constexpr QLatin1StringView KEY_DATABITS  {"uart/databits"};
    inline static constexpr QLatin1StringView KEY_STOPBITS  {"uart/stopbits"};
    inline static constexpr QLatin1StringView KEY_PARITY    {"uart/parity"};
    inline static constexpr QLatin1StringView KEY_LINE_END  {"uart/lineEnding"};

    inline static constexpr QLatin1StringView KEY_TIMESTAMP {"view/timestamp"};
    inline static constexpr QLatin1StringView KEY_HEX       {"view/hex"};

    inline static constexpr QLatin1StringView PARITY_NONE {"None"};
    inline static constexpr QLatin1StringView PARITY_EVEN {"Even"};
    inline static constexpr QLatin1StringView PARITY_ODD  {"Odd"};

    inline static constexpr QLatin1StringView LINE_LF   {"LF"};
    inline static constexpr QLatin1StringView LINE_CRLF {"CRLF"};

    static constexpr int DEF_BAUDRATE = 9600;
    static constexpr QSerialPort::DataBits DEF_DATABITS = QSerialPort::Data8;
    static constexpr QSerialPort::StopBits DEF_STOPBITS = QSerialPort::OneStop;
    inline static constexpr QLatin1StringView DEF_DEVICE {"/dev/ttyUSB0"};

    static constexpr int DEF_HEARTBEAT_INTERVAL = 1000; // Milliseconds
    static constexpr int HEARTBEAT_TIMEOUT = 2000; // Milliseconds

    void ensureDefaults();
    void loadUiFromSettings();
    void reconnect();
    void checkConnectionHealth();

    void appendMessage(const QByteArray &data,
                       QLatin1StringView prefix,
                       const QColor &color);
    void appendRx(const QByteArray &data);
    void appendTx(const QByteArray &data);
    QSerialPort::Parity parityFromString(QStringView p) const;

    void setConnectionState(ConnectionState state);
    inline static QString connectionStateText(ConnectionState state);

    std::optional<ConnectionState> connectionState;

private:
    Ui::MainWindow *ui;
    QSerialPort serial;
    QSettings settings;
    QTimer *heartbeatTimer;
    QByteArray rxBuffer;
    QElapsedTimer lastRx;

signals:
    void connectionStatusChanged(MainWindow::ConnectionState state);
};

#endif // MAINWINDOW_H
