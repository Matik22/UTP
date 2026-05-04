#include "mainwindow.h"
#include "logindialog.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Catalog catalog;
    catalog.loadLibrarians();
    catalog.loadData();

    LoginDialog login(catalog);
    if (login.exec() != QDialog::Accepted)
        return 0;

    MainWindow w;
    w.show();
    return a.exec();
}
