#include "logindialog.h"
#include "catalog.h"
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>

LoginDialog::LoginDialog(Catalog& catalog, QWidget* parent)
    : QDialog(parent), m_catalog(catalog)
{
    setWindowTitle("Вход библиотекаря");
    setModal(true);
    resize(300, 150);

    QVBoxLayout* layout = new QVBoxLayout(this);

    m_loginEdit = new QLineEdit(this);
    m_loginEdit->setPlaceholderText("Логин");

    m_passEdit = new QLineEdit(this);
    m_passEdit->setPlaceholderText("Пароль");
    m_passEdit->setEchoMode(QLineEdit::Password);

    QPushButton* btn = new QPushButton("Войти", this);

    layout->addWidget(m_loginEdit);
    layout->addWidget(m_passEdit);
    layout->addWidget(btn);

    connect(m_loginEdit, &QLineEdit::returnPressed, this, [this]() {
        m_passEdit->setFocus();
    });

    connect(m_passEdit, &QLineEdit::returnPressed, this, [this]() {
        if (m_passEdit->text().isEmpty()) {
            m_loginEdit->setFocus();
        } else {
            onLogin();
        }
    });
}

void LoginDialog::onLogin() {
    QString login = m_loginEdit->text().trimmed();
    QString pass  = m_passEdit->text().trimmed();

    if (m_catalog.checkLibrarian(login.toStdString(), pass.toStdString())) {
        accept();
    } else {
        QMessageBox::warning(this, "Ошибка", "Неверный логин или пароль");
    }
}
