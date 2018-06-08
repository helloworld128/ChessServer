#include "server.h"
#include <QTcpSocket>
#include "ui_server.h"
#include <QMessageBox>
#include <QStringListModel>
int Server::Game::count = 1000;

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
    print("hello!");
    connect(server, SIGNAL(newConnection()), this, SLOT(newConnection()));
}

void Server::newConnection(){
    auto tmp = server->nextPendingConnection();
    QString str = "incoming connection:" + tmp->peerAddress().toString();
    print(str);
    connect(tmp, SIGNAL(readyRead()), this, SLOT(processData()));
    connect(tmp, SIGNAL(disconnected()), this, SLOT(deleteConnection()));
    connections[tmp] = nullptr;
}

void Server::processData(){
    auto s = dynamic_cast<QTcpSocket*>(sender());
    QByteArray content = s->readAll();
    QByteArray ba;
    QDataStream in(&content, QIODevice::ReadOnly);
    QDataStream out(&ba, QIODevice::WriteOnly);
    QChar t; in >> t;
    print(t);
    char type = t.toLatin1();
    switch(type){
    case 's':

        break;
    case 'g':
        out << QChar('g');
        out << gameList.size();
        foreach (Game* g, gameList){
            out << g->type << g->uid << g->name[0] << g->name[1];
        }
        s->write(ba);
        break;
    case 'c':
    {
        int type, side; QString name;
        in >> type >> side >> name;
        Game* game = new Game;
        game->uid = Game::count++;
        game->name[side] = name;
        game->socket[side] = s;
        game->type = type;
        gameList.push_back(game);
        connections[s] = game;
        break;
    }
    case 'r':
    {
        Game* g = connections[s];
        if (g->oneReady){
            out << QChar('s');
            s->write(ba);
            g->otherSocket(s)->write(ba);
        }
        else{
            g->oneReady = true;
        }
        break;
    }
    case 'p':
    {
        int x, y;
        in >> x >> y;
        out << QChar('p') << x << y;
        connections[s]->otherSocket(s)->write(ba);
        break;
    }
    case 'j':
    {
        int id, side;
        QString name;
        in >> id >> side >> name;
        foreach (Game* g, gameList){
            if (g->uid == id){
                g->socket[side] = s;
                g->name[side] = name;
                connections[s] = g;
                break;
            }
        }
        out << QChar('e') << name;
        connections[s]->otherSocket(s)->write(ba);
        break;
    }
    case 'l':
    {
        out << QChar('l');
        Game* g = connections[s];
        g->otherSocket(s)->write(ba);
        connections[g->socket[0]] = nullptr;
        connections[g->socket[1]] = nullptr;
        delete connections[s];
        gameList.removeOne(connections[s]);
        break;
    }
    default:
        print("Unknown command!");
        break;
    }
    //    QString c; int i;QChar b;
    //    in >> c >> i >> b;
    //    print(content);
    //    auto content = content.data();
    //    switch (content[0]){
    //    case 'g':
    //    {
    //        int n = gameList.size();
    //        char* text = new char[dataSize * n + 1];
    //        text[0] = 'g';

    //        break;
    //    }
    //    case 'r': case 'p':
    //        connections[s]->otherSocket(s)->write(content);
    //        break;
    //    case 'j':
    //    {
    //        int id = 1000 * (content[1] - '0') + 100 * (content[2] - '0')
    //                + 10 * (content[3] - '0') + (content[4] - '0');
    //        char* name = content + 5;
    //        foreach (auto game, gameList)
    //        {
    //            if (game.uid == id){
    //                char tmp[12];
    //                tmp[0] = 'e'; strcpy(tmp + 1, name);
    //                if (game.socket_1 == nullptr){
    //                    game.socket_1 = s;
    //                    game.name_1 = name;
    //                    game.socket_2->write(tmp);
    //                }
    //                else{
    //                    game.socket_2 = s;
    //                    game.name_2 = name;
    //                    game.socket_1->write(tmp);
    //                }
    //            }
    //            break;
    //        }
    //        break;
    //    }
    //    case 'l':
    //        break;
    //    case 'c':
    //    {
    //        Game* game = new Game;

    //        break;
    //    }
    //    default:
    //        print("unknown command!");
    //        break;
    //    }
}

void Server::deleteConnection(){
    auto s = dynamic_cast<QTcpSocket*>(sender());
    print("removing a connection...");
    connections.remove(s);
}

void Server::print(QString text){
    ui->listWidget->addItem(text);
}

Server::~Server()
{
    delete ui;
}

QTcpSocket* Server::Game::otherSocket(QTcpSocket* socket){
    if (socket == Game::socket[0]) return Game::socket[1];
    else return Game::socket[0];
}
