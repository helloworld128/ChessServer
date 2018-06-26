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
        QChar f('a');
        in >> x >> y >> f;
        //after a client sends a 'p'(put), it will soon check whether the game is finished, and
        //if so, it will send an 'f'. The interval is so short that the server can't recognize them as
        //seperate messages.
        if (f.toLatin1() == 'f') connections[s]->oneReady = false;
        out << QChar('p') << x << y;
        connections[s]->otherSocket(s)->write(ba);
        foreach (QTcpSocket* sock, connections[s]->spectators) {
            sock->write(ba);
        }
        break;
    }
    case 'w'://watch
    {
        int id;
        in >> id;
        foreach (Game* g, gameList){
            if (g->uid == id){
                connections[s] = g;
                g->spectators.push_back(s);
                out << QChar('z');
                g->socket[0]->write(ba);
                break;
            }
        }
        break;
    }
    case 'z':
    {
        foreach (QTcpSocket* sock, connections[s]->spectators){
            sock->write(content);
        }
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
    case 'q':
    {
        Game* g = connections[s];
        foreach (auto sock, connections[s]->spectators)
            if (s == sock) return;
        if (g == nullptr) break;
        if (g->otherSocket(s) != nullptr){
            out << QChar('q');
            g->otherSocket(s)->write(ba);
        }
        connections[g->socket[0]] = nullptr;
        connections[g->socket[1]] = nullptr;
        delete g;
        gameList.removeOne(g);
        break;
    }
    case 'u':
    {
        out << QChar('u');
        connections[s]->otherSocket(s)->write(ba);
        break;
    }
    case 'o':
    {
        out << QChar('o');
        connections[s]->otherSocket(s)->write(ba);
        break;
    }
    case 't':
    {
        if (connections[s]->otherSocket(s) == nullptr) break;
        QString name, text;
        in >> name >> text;
        out << QChar('t') << name << text;
        connections[s]->otherSocket(s)->write(ba);
        break;
    }
    case 'f':
        connections[s]->oneReady = false;
        break;
    default:
        print("Unknown command!");
        break;
    }
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
