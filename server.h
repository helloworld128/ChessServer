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
    void readData();
    void deleteConnection();

private:
    Ui::Server *ui;
    QTcpServer* server = nullptr;
    QList<QTcpSocket*> connections;
    QStringList data;
};

#endif // SERVER_H
