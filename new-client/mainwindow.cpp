#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    socket = new QTcpSocket();
    nextBlockSize = 0;
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::readyRead);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_connectBtn_clicked()
{
    socket->connectToHost("127.0.0.1", 5555);
}

// Когда пришли данные от сервера
void MainWindow::readyRead()
{
    QDataStream in(socket); // поток для хранения данных отправленных сокетом
    in.setVersion(QDataStream::Qt_5_15);

    // Бесконечный цикл, чтобы клиент был в состоянии обрабатывать как весь блок целиком, так и часть
    for (;;){
        // Размер блока неивзестен (он по умолчанию == 0)
        if(nextBlockSize == 0){
            // размер блока не меньше двух байт
            if (socket->bytesAvailable() < 2){
                break;
            }
            in >> nextBlockSize; // считываем размер блока
        }
        // Если фактический размер блока меньше заявленного,значит данные пришли не целиком
        if (socket->bytesAvailable() < nextBlockSize){
            break;
        }
        // Если все ок
        QString str; // результирующая строка
        in >> str; // через оператор >> десериализуем данные в строку
        nextBlockSize = 0; // обнуляем для следующего соединения
        ui->textBrowser->append(str);
    }
}

void MainWindow::sendToServer(QString str)
{
    data.clear();
    QDataStream out(&data, QIODevice::WriteOnly); // создаем поток out и указываем data как буфер(данные запишутся в data)
    out.setVersion(QDataStream::Qt_5_15); // без указания версии крашится

    out << qint16(0); // резервируем место для хранения размера сообщения
    out << str; // само сообщение (сериализуем в строку)
    out.device()->seek(0); // возвращаемся в начало потока (т.е сообщение начинается с самого начала, игнорируя эти 2 зарезервированных байта)
    out << qint16(data.size() - sizeof(qint16)); // получаем фактический размер сообщения вычитая 2 зарезервированных байта

    socket->write(data);
    ui->lineEdit->clear();
}

void MainWindow::on_sendMessageBtn_clicked()
{
    sendToServer(ui->lineEdit->text());
}

void MainWindow::on_lineEdit_returnPressed()
{
    sendToServer(ui->lineEdit->text());
}

