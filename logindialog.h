#pragma once
#include <QDialog>

class QLineEdit;
class Catalog;

class LoginDialog : public QDialog {
    Q_OBJECT
public:
    explicit LoginDialog(Catalog& catalog, QWidget* parent = nullptr);

private slots:
    void onLogin();

private:
    Catalog& m_catalog;
    QLineEdit* m_loginEdit;
    QLineEdit* m_passEdit;
};
