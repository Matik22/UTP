#include "issuedialog.h"
#include "catalog.h"
#include <QFormLayout>
#include <QComboBox>
#include <QDateEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QDialogButtonBox>

constexpr int kDefaultBorrowDays = 14;

IssueDialog::IssueDialog(Catalog& catalog, QWidget* parent)
    : QDialog(parent), m_catalog(catalog), m_bookCombo(nullptr), m_userCombo(nullptr),
    m_issueDateEdit(nullptr), m_returnDateEdit(nullptr) {
    setupUI();
}

IssueDialog::~IssueDialog() = default;

void IssueDialog::setupUI() {
    setWindowTitle("Оформление выдачи книги");
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setModal(true);
    resize(380, 280);

    QFormLayout* formLayout = new QFormLayout(this);

    m_bookCombo = new QComboBox(this);
    m_userCombo = new QComboBox(this);

    m_issueDateEdit = new QDateEdit(this);
    m_issueDateEdit->setCalendarPopup(true);
    m_issueDateEdit->setDate(QDate::currentDate());

    m_returnDateEdit = new QDateEdit(this);
    m_returnDateEdit->setCalendarPopup(true);
    m_returnDateEdit->setDate(QDate::currentDate().addDays(kDefaultBorrowDays));
    m_returnDateEdit->setReadOnly(false);

    populateBooks();
    populateUsers();

    formLayout->addRow(QStringLiteral("Доступная книга:"), m_bookCombo);
    formLayout->addRow(QStringLiteral("Читатель:"), m_userCombo);
    formLayout->addRow(QStringLiteral("Дата выдачи:"), m_issueDateEdit);
    formLayout->addRow(QStringLiteral("Срок возврата:"), m_returnDateEdit);

    QDialogButtonBox* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    formLayout->addRow(btnBox);
    connect(btnBox, &QDialogButtonBox::accepted, this, &IssueDialog::onIssueConfirmed);
    connect(btnBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_issueDateEdit, &QDateEdit::dateChanged, this, &IssueDialog::onIssueDateChanged);
}

void IssueDialog::populateBooks() {
    m_bookCombo->clear();
    const auto& books = m_catalog.getAvailableBooks();
    for (const auto& b : books) {
        QString text = QString::fromStdString(b.getTitle() + " (" + b.getAuthor() + ")");
        m_bookCombo->addItem(text, QString::fromStdString(b.getBookId()));
    }
}

void IssueDialog::populateUsers() {
    m_userCombo->clear();
    const auto& users = m_catalog.getUsers();
    for (const auto& u : users) {
        QString text = QString::fromStdString(u.getFullName() + " [" + u.getUserId() + "]");
        m_userCombo->addItem(text, QString::fromStdString(u.getUserId()));
    }
}

void IssueDialog::onIssueDateChanged(const QDate& date) {
    m_returnDateEdit->setDate(date.addDays(LibraryConstants::kMaxBorrowDays));
}

void IssueDialog::onIssueConfirmed() {
    QString bookId = m_bookCombo->currentData().toString();
    QString userId = m_userCombo->currentData().toString();

    if (bookId.isEmpty() || userId.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Выберите книгу и читателя.");
        return;
    }

    bool ok = m_catalog.issueBook(bookId.toStdString(), userId.toStdString(),
                                  m_issueDateEdit->date().toString("yyyy-MM-dd").toStdString(),
                                  m_returnDateEdit->date().toString("yyyy-MM-dd").toStdString());
    if (ok) {
        accept();
    } else {
        QMessageBox::warning(this, "Ошибка выдачи", "Не удалось оформить выдачу.");
    }
}
