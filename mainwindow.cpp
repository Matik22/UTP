#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QStatusBar>
#include <QDebug>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QDate>
#include <algorithm>
#include "readerdialog.h" // ← Добавить в начало
#include <QInputDialog>
#include <QLabel>


MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), m_sourceModel(nullptr), m_proxyModel(nullptr) {
    setupUI();
    m_catalog.loadData();
    refreshTable();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI() {
    setWindowTitle("АРМ Библиотекаря | Учёт фонда");
    resize(1050, 650);

    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    QHBoxLayout* topLayout = new QHBoxLayout();
    m_btnAdd = new QPushButton("Добавить книгу");
    m_btnSave = new QPushButton("Сохранить");
    m_btnDeleteBook = new QPushButton("Удалить книгу");
    topLayout->addWidget(m_btnDeleteBook);
    connect(m_btnDeleteBook, &QPushButton::clicked, this, &MainWindow::onRemoveBookById);
    // В setupUI(), после создания m_btnSave:
    m_btnReaders = new QPushButton("Читатели");
    topLayout->addWidget(m_btnReaders);
    connect(m_btnReaders, &QPushButton::clicked, this, &MainWindow::openReaderMenu);
    topLayout->addWidget(m_btnAdd);
    topLayout->addStretch();
    topLayout->addWidget(m_btnSave);
    mainLayout->addLayout(topLayout);

    m_tableView = new QTableView();
    m_sourceModel = new QStandardItemModel(this);
    m_sourceModel->setHorizontalHeaderLabels({"ID", "Название", "Автор", "Издательство", "Год", "Жанр"});

    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_sourceModel);
    m_proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);

    m_tableView->setModel(m_proxyModel);
    m_tableView->setSortingEnabled(true);
    m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    mainLayout->addWidget(m_tableView);

    connect(m_btnAdd, &QPushButton::clicked, this, &MainWindow::onAddBook);
    connect(m_btnSave, &QPushButton::clicked, this, &MainWindow::onSaveData);

    statusBar()->showMessage("Система готова. Кликните по заголовку для сортировки.");
}

void MainWindow::refreshTable() {
    m_sourceModel->removeRows(0, m_sourceModel->rowCount());
    const auto& books = m_catalog.getBooks();

    qDebug() << "[DEBUG] В каталоге книг:" << books.size();

    if (books.empty()) {
        qDebug() << "[INFO] Каталог пуст.";
        QList<QStandardItem*> row;
        row << new QStandardItem("DEMO-001") << new QStandardItem("Основы C++")
            << new QStandardItem("Липпман С.") << new QStandardItem("Питер")
            << new QStandardItem("2021") << new QStandardItem("Учебник");
        m_sourceModel->appendRow(row);
        return;
    }

    for (const auto& b : books) {
        QList<QStandardItem*> row;
        row << new QStandardItem(QString::fromStdString(b.getBookId()))
            << new QStandardItem(QString::fromStdString(b.getTitle()))
            << new QStandardItem(QString::fromStdString(b.getAuthor()))
            << new QStandardItem(QString::fromStdString(b.getPublisher()))
            << new QStandardItem(QString::number(b.getYear()))
            << new QStandardItem(QString::fromStdString(b.getGenre()));
        m_sourceModel->appendRow(row);
    }
}

void MainWindow::onAddBook() {
    QDialog dialog(this);
    dialog.setWindowTitle("Регистрация издания");
    dialog.setModal(true);
    dialog.setMinimumWidth(350);

    QFormLayout* formLayout = new QFormLayout(&dialog);

    QLineEdit* titleEdit = new QLineEdit(&dialog);
    QLineEdit* authorEdit = new QLineEdit(&dialog);
    QLineEdit* publisherEdit = new QLineEdit(&dialog);
    QSpinBox* yearEdit = new QSpinBox(&dialog);
    yearEdit->setRange(1000, QDate::currentDate().year());
    yearEdit->setValue(QDate::currentDate().year());
    QLineEdit* genreEdit = new QLineEdit(&dialog);
    formLayout->addRow(QStringLiteral("Название:"), titleEdit);
    formLayout->addRow(QStringLiteral("Автор:"), authorEdit);
    formLayout->addRow(QStringLiteral("Издательство:"), publisherEdit);
    formLayout->addRow(QStringLiteral("Год издания:"), yearEdit);
    formLayout->addRow(QStringLiteral("Жанр:"), genreEdit);

    QDialogButtonBox* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    formLayout->addRow(btnBox);
    connect(btnBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(btnBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        QString title = titleEdit->text().trimmed();
        QString author = authorEdit->text().trimmed();
        QString publisher = publisherEdit->text().trimmed();
        int year = yearEdit->value();
        QString genre = genreEdit->text().trimmed();

        if (title.isEmpty() || author.isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Название и Автор обязательны.");
            return;
        }

        // Генерация неизменяемого ID
        std::string autoId = m_catalog.generateBookId(genre.toStdString());
        Book newBook(autoId, title.toStdString(), author.toStdString(),
                     publisher.toStdString(), year, genre.toStdString());

        m_catalog.addBook(newBook);
        refreshTable();
        QMessageBox::information(this, "Успех", QString("Издание добавлено. Присвоен ID: %1").arg(QString::fromStdString(autoId)));
    }
}

void MainWindow::onRemoveBookById() {
    bool ok;
    QString bookId = QInputDialog::getText(this, "Удаление издания",
                                           "Введите ID книги для удаления:", QLineEdit::Normal, "", &ok);
    if (!ok || bookId.trimmed().isEmpty()) return;

    if (m_catalog.removeBook(bookId.trimmed().toStdString())) {
        refreshTable();
        QMessageBox::information(this, "Успех", "Книга удалена из каталога.");
    } else {
        QMessageBox::warning(this, "Ошибка",
                             "Книга не найдена или находится в обороте у читателя. Удаление невозможно.");
    }
}

void MainWindow::onSaveData() {
    m_catalog.saveData();
    QMessageBox::information(this, "Успех", "Данные сохранены в library_data.txt");
}

// В конец файла:
void MainWindow::openReaderMenu() {
    ReaderMenuDialog dialog(m_catalog, this);
    dialog.exec();
    // После закрытия меню можно обновить главные данные если нужно
}