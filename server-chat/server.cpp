#include "server.h"

// В конструкторе просто запуск сервера
Server::Server()
{
    if (this->listen(QHostAddress::LocalHost, 5555)){
        qDebug() << "Listening";
    } else {
        qDebug() << "Error: " << errorString();
    }
    nextBlockSize = 0;
}

void Server::incomingConnection(qintptr socketDescriptor)
{
    socket = new QTcpSocket();
    socket->setSocketDescriptor(socketDescriptor);
    connect(socket, &QTcpSocket::readyRead, this, &Server::readyRead);
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);

    qDebug() << "client connected" << socketDescriptor;

    sockets.push_back(socket);
}

void Server::readyRead()
{
    // Сокет отправивший сигнал
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (socket) {
        QDataStream in(socket); // in хранит в себе отправленные данные сокета (in т.к. считываем данные)
        in.setVersion(QDataStream::Qt_5_15);
        if (in.status() == QDataStream::Ok){
            // Бесконечный цикл, чтобы клиент был в состоянии обрабатывать как весь блок целиком, так и часть
            for (;;){
                // Размер блока неивзестен (он по умолчанию == 0)
                if(nextBlockSize == 0){
                    qDebug() <<"nextBlockSize == 0";
                    // размер блока не меньше двух зарезервированных байт
                    if (socket->bytesAvailable() < 2){
                        qDebug() <<"bytesAvailable() < 2";
                        break;
                    }
                    in >> nextBlockSize; // считываем размер блока
                }
                // Если фактический размер блока меньше заявленного,значит данные пришли не целиком
                if (socket->bytesAvailable() < nextBlockSize){
                    qDebug() <<"bytesAvailable() < nextBlockSize -> dataNotFull";
                    break;
                }
                // Если все ок
                QString str; // результирующая строка
                in >> str; // через оператор >> десериализуем данные в строку
                nextBlockSize = 0; // обнуляем для следующего соединения
                sendToClient(str);
                qDebug() << str;
                break;
            }

        } else {
            qDebug() << "data Stream error";
        }

    } else {
        qDebug() << "Error: sender() is not a QTcpSocket";
    }
}

void Server::sendToClient(QString str)
{
    data.clear();
    QString time = QTime::currentTime().toString("HH:mm:ss");
    QString messageWithTime = time + ": " + str;
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);

    out << qint16(0); // резервируем место для хранения размера сообщения
    out << messageWithTime; // само сообщение (сериализуем в строку)
    out.device()->seek(0); // возвращаемся в начало потока (т.е сообщение начинается с самого начала, игнорируя эти 2 зарезервированных байта)

    // Отправляем сообщение всем подключенным клиентам
    for (int i = 0; i < sockets.size(); ++i) {
        sockets[i]->write(data);
    }
}
