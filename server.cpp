#include "server.h"
#include <QTcpSocket>
#include "ui_server.h"
#include <QMessageBox>
#include <QStringListModel>

Server::Server(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Server)
{
    ui->setupUi(this);
    server = new QTcpServer(this);
    if (!server->listen(QHostAddress::AnyIPv4, 23333)) {
            QMessageBox::critical(this, tr("Server"),
                                  tr("Unable to start the server: %1.")
                                  .arg(server->errorString()));
            close();
            return;
        }
    connect(server, SIGNAL(newConnection()), this, SLOT(newConnection()));
    QStringListModel* model = new QStringListModel(data);
    ui->listView->setModel(model);
}

void Server::newConnection(){
    auto tmp = server->nextPendingConnection();
    connect(tmp, SIGNAL(readyRead()), this, SLOT(readData()));
    connect(tmp, SIGNAL(disconnected()), this, SLOT(deleteConnection()));
    connections.push_back(tmp);
}

void Server::readData(){
    auto s = dynamic_cast<QTcpSocket*>(sender());
    auto content = s->readAll().toStdString();
    data << QString::fromStdString(content);
    ui->label->setText(QString::fromStdString(content));
}

void Server::deleteConnection(){
    auto s = dynamic_cast<QTcpSocket*>(sender());
    connections.removeOne(s);
}

Server::~Server()
{
    delete ui;
}
