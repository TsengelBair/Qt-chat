#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDataStream>
#include <QTime>

class Server : public QTcpServer
{
    Q_OBJECT
public:
    Server();
    QTcpSocket* socket; // можно было и не делать членом класса, но пока пусть будет так

private:
    QVector<QTcpSocket*>sockets;
    QByteArray data;
    qint16 nextBlockSize;
    void sendToClient(QString str);

public slots:
    void incomingConnection(qintptr socketDescriptor); // идентификатор сокета
    void readyRead();
};

#endif // SERVER_H
