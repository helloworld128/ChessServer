#ifndef SERVER_H
#define SERVER_H

#include <QWidget>
#include <QTcpServer>
#include <QList>

namespace Ui {
class Server;
}

class Server : public QWidget
{
    Q_OBJECT

public:
    explicit Server(QWidget *parent = 0);
    ~Server();

private slots:
    void newConnection();
    void processData();
    void deleteConnection();

private:
    Ui::Server *ui;
    class Game;
    void print(QString text);
    void removeGame(QTcpSocket* s);
    QTcpServer* server = nullptr;
    QMap<QTcpSocket*, Game*> connections;
    QVector<Game*> gameList;
};

class Server::Game{
public:
    QVector<QTcpSocket*> spectators;
    static int count;
    QTcpSocket* otherSocket(QTcpSocket* socket);
    int type;
    int uid;
    QTcpSocket* socket[2] = {nullptr, nullptr};
    QString name[2];
    bool oneReady = false;
};

#endif // SERVER_H
