#include "readerdialog.h"
#include "catalog.h"
#include "user.h"          // ← Добавлено
#include "issuerecord.h"   // ← Добавлено
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableView>
#include <QPushButton>
#include <QLineEdit>
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>
#include <algorithm>       // ← Для std::find_if

ReaderMenuDialog::ReaderMenuDialog(Catalog& catalog, QWidget* parent)
    : QDialog(parent), m_catalog(catalog), m_userModel(nullptr), m_userProxy(nullptr), m_historyModel(nullptr) {
    setupUI();
    outputReader();
}

ReaderMenuDialog::~ReaderMenuDialog() = default; // ← Исправлено имя

void ReaderMenuDialog::setupUI() {
    setWindowTitle("Меню работы с читателями");
    resize(950, 550);
    setModal(true);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QHBoxLayout* topLayout = new QHBoxLayout();

    m_btnRegister = new QPushButton("Регистрация читателя");
    m_btnDelete   = new QPushButton("Удалить по фамилии");
    m_surnameEdit = new QLineEdit();
    m_surnameEdit->setPlaceholderText("Введите фамилию...");
    m_surnameEdit->setMaximumWidth(220);

    topLayout->addWidget(m_btnRegister);
    topLayout->addWidget(m_btnDelete);
    topLayout->addWidget(m_surnameEdit);
    topLayout->addStretch();
    mainLayout->addLayout(topLayout);

    QHBoxLayout* tablesLayout = new QHBoxLayout();
    m_userTable = new QTableView();
    m_userModel = new QStandardItemModel(this);
    m_userModel->setHorizontalHeaderLabels({"ID", "ФИО", "Телефон"});
    m_userProxy = new QSortFilterProxyModel(this);
    m_userProxy->setSourceModel(m_userModel);
    m_userTable->setModel(m_userProxy);
    m_userTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_userTable->setAlternatingRowColors(true);
    tablesLayout->addWidget(m_userTable, 2);

    m_historyTable = new QTableView();
    m_historyModel = new QStandardItemModel(this);
    m_historyModel->setHorizontalHeaderLabels({"ID книги", "Дата выдачи", "Дата возврата", "Статус"});
    m_historyTable->setModel(m_historyModel);
    m_historyTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_historyTable->setAlternatingRowColors(true);
    tablesLayout->addWidget(m_historyTable, 1);

    mainLayout->addLayout(tablesLayout);

    connect(m_btnRegister, &QPushButton::clicked, this, &ReaderMenuDialog::onRegisterReader);
    connect(m_btnDelete, &QPushButton::clicked, this, &ReaderMenuDialog::onRemoveReaderBySurname);
    connect(m_userTable->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &ReaderMenuDialog::onReaderSelectionChanged);
}

void ReaderMenuDialog::outputReader() {
    m_userModel->removeRows(0, m_userModel->rowCount());
    const auto& users = m_catalog.getUsers();
    for (const auto& u : users) {
        QList<QStandardItem*> row;
        row << new QStandardItem(QString::fromStdString(u.getUserId()))
            << new QStandardItem(QString::fromStdString(u.getFullName()))
            << new QStandardItem(QString::fromStdString(u.getPhoneNumber()));
        m_userModel->appendRow(row); // ← Исправлен вызов для Qt 5.15
    }
}

void ReaderMenuDialog::issueHistory(const std::string& userId) {
    m_historyModel->removeRows(0, m_historyModel->rowCount());
    auto history = m_catalog.getUserHistory(userId);
    for (const auto& rec : history) {
        QList<QStandardItem*> row;
        row << new QStandardItem(QString::fromStdString(rec.getBookId()))
            << new QStandardItem(QString::fromStdString(rec.getIssueDate()))
            << new QStandardItem(QString::fromStdString(rec.getReturnDate().empty() ? "В обороте" : rec.getReturnDate()))
            << new QStandardItem(rec.getIsOverdue() ? "Просрочка" : "В норме");
        m_historyModel->appendRow(row);
    }
}

void ReaderMenuDialog::onRegisterReader() {
    bool ok;
    QString fullName = QInputDialog::getText(this, "Регистрация", "ФИО читателя:", QLineEdit::Normal, "", &ok);
    if (!ok || fullName.trimmed().isEmpty()) return;

    QString phone = QInputDialog::getText(this, "Регистрация", "Номер телефона:", QLineEdit::Normal, "+7", &ok);

    // Автоматическая генерация неизменяемого ID
    std::string autoId = m_catalog.generateUserId();
    m_catalog.addUser(User(autoId, fullName.trimmed().toStdString(), phone.trimmed().toStdString()));

    outputReader();
    QMessageBox::information(this, "Успех", QString("Читатель зарегистрирован.")
                                                .arg(QString::fromStdString(autoId)));
}

void ReaderMenuDialog::onRemoveReaderBySurname() {
    QString surname = m_surnameEdit->text().trimmed().toLower();
    if (surname.isEmpty()) {
        QMessageBox::warning(this, "Ошибка ввода", "Введите фамилию для поиска.");
        return;
    }

    const auto& users = m_catalog.getUsers();
    auto it = std::find_if(users.begin(), users.end(), [&](const User& u){
        return QString::fromStdString(u.getFullName()).toLower().contains(surname);
    });

    if (it != users.end()) {
        m_catalog.removeUser(it->getUserId());
        outputReader();
        m_historyModel->removeRows(0, m_historyModel->rowCount());
        QMessageBox::information(this, "Успех", "Читатель удалён.");
    } else {
        QMessageBox::information(this, "Информация", "Читатель с такой фамилией не найден.");
    }
}

void ReaderMenuDialog::onReaderSelectionChanged(const QModelIndex& current, const QModelIndex&) {
    if (!current.isValid()) {
        m_historyModel->removeRows(0, m_historyModel->rowCount());
        return;
    }
    QString userId = m_userModel->item(current.row(), 0)->text();
    issueHistory(userId.toStdString());
}