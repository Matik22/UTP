#include "mainwindow.h"
#include "logindialog.h"
#include <QApplication>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    QLocale::setDefault(QLocale(QLocale::Russian, QLocale::Russia));

    Catalog catalog;
    catalog.loadLibrarians();
    catalog.loadData();
    catalog.loadReaders();
    catalog.loadIssueRecords();

    LoginDialog login(catalog);
    if (login.exec() != QDialog::Accepted)
        return 0;

    MainWindow w;
    w.show();
    return a.exec();
}
