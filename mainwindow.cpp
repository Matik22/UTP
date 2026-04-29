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
#include <QInputDialog>
#include <QLabel>
#include <QComboBox>
#include "issuedialog.h"
#include "piechartwidget.h"
#include "user.h"

MainWindow::MainWindow(QWidget* parent)
 : QMainWindow(parent), m_sourceModel(nullptr), m_proxyModel(nullptr),
   m_searchEdit(nullptr), m_searchColumn(nullptr),
   m_userModel(nullptr), m_userProxy(nullptr), m_historyModel(nullptr) {
 setupUI();
 m_catalog.loadData();
 refreshTable();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI() {
 setWindowTitle("АРМ Библиотекаря | Учёт фонда");
 resize(1050, 650);

 m_stack = new QStackedWidget(this);
 setCentralWidget(m_stack);

 setupBooksPage();
 setupReadersPage();

 setStyleSheet(
  "QMainWindow, QWidget { background-color: #2b2b2b; color: #ffffff; }"
  "QTableView { background-color: #3c3c3c; color: #ffffff; border: 1px solid #555; gridline-color: #555; alternate-background-color: #333333; }"
  "QTableView::item { color: #ffffff; }"
  "QTableView::item:selected { background-color: #0d47a1; }"
  "QHeaderView::section { background-color: #2b2b2b; color: #cccccc; padding: 6px; border: none; border-bottom: 1px solid #555; }"
  "QPushButton { background-color: #0d47a1; color: #ffffff; border: none; border-radius: 3px; padding: 6px 12px; font-weight: bold; }"
  "QPushButton:hover { background-color: #1565c0; }"
  "QPushButton:pressed { background-color: #0a3570; }"
  "QPushButton:disabled { background-color: #444444; color: #888888; }"
  "QLineEdit { background-color: #3c3c3c; color: #ffffff; border: 1px solid #555; border-radius: 3px; padding: 4px 8px; }"
  "QLineEdit::placeholder { color: #888888; }"
  "QComboBox { background-color: #3c3c3c; color: #ffffff; border: 1px solid #555; border-radius: 3px; padding: 4px 8px; }"
  "QComboBox::drop-down { border: none; }"
  "QComboBox QAbstractItemView { background-color: #3c3c3c; color: #ffffff; selection-background-color: #0d47a1; }"
  "QComboBox::down-arrow { image: none; border: none; }"
  "QLabel { color: #cccccc; }"
  "QStatusBar { background-color: #2b2b2b; color: #aaaaaa; }"
  "QScrollBar:vertical { background-color: #2b2b2b; width: 10px; }"
  "QScrollBar::handle:vertical { background-color: #555; border-radius: 4px; min-height: 20px; }"
  "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
  "QScrollBar:horizontal { background-color: #2b2b2b; height: 10px; }"
  "QScrollBar::handle:horizontal { background-color: #555; border-radius: 4px; min-width: 20px; }"
  "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }"
 );

 statusBar()->showMessage("Система готова. Кликните по заголовку для сортировки.");
}

void MainWindow::setupBooksPage() {
 m_booksPage = new QWidget();
 QVBoxLayout* mainLayout = new QVBoxLayout(m_booksPage);

 QHBoxLayout* topLayout = new QHBoxLayout();
 m_btnDeleteBook = new QPushButton("Удалить книгу");
 topLayout->addWidget(m_btnDeleteBook);
 QObject::connect(m_btnDeleteBook, &QPushButton::clicked, this, &MainWindow::onRemoveBookById);

 m_btnReaders = new QPushButton("Читатели");
 topLayout->addWidget(m_btnReaders);
 QObject::connect(m_btnReaders, &QPushButton::clicked, this, &MainWindow::onSwitchToReaders);

 m_btnAdd = new QPushButton("Добавить книгу");
 topLayout->addWidget(m_btnAdd);

 m_btnIssueBook = new QPushButton("Выдать книгу");
 topLayout->addWidget(m_btnIssueBook);
 QObject::connect(m_btnIssueBook, &QPushButton::clicked, this, &MainWindow::onIssueBook);

 m_btnChart = new QPushButton("Диаграмма");
 topLayout->addWidget(m_btnChart);
 QObject::connect(m_btnChart, &QPushButton::clicked, this, &MainWindow::onShowChart);

 topLayout->addStretch();
 m_btnSave = new QPushButton("Сохранить");
 topLayout->addWidget(m_btnSave);
 mainLayout->addLayout(topLayout);

 // Строка поиска
 QHBoxLayout* searchLayout = new QHBoxLayout();
 m_searchColumn = new QComboBox();
 m_searchColumn->addItems({"ID", "Название", "Автор", "Издательство", "Год", "Жанр"});
 m_searchEdit = new QLineEdit();
 m_searchEdit->setPlaceholderText("Введите текст для поиска...");
 searchLayout->addWidget(new QLabel("Искать по:"));
 searchLayout->addWidget(m_searchColumn);
 searchLayout->addWidget(m_searchEdit);
 mainLayout->addLayout(searchLayout);

 QObject::connect(m_searchEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchChanged);
 QObject::connect(m_searchColumn, QOverload<int>::of(&QComboBox::currentIndexChanged),
         this, &MainWindow::onSearchChanged);

 m_tableView = new QTableView();
 m_sourceModel = new QStandardItemModel(this);
 m_sourceModel->setHorizontalHeaderLabels({"ID", "Название", "Автор", "Издательство", "Год", "Жанр"});

 m_proxyModel = new QSortFilterProxyModel(this);
 m_proxyModel->setSourceModel(m_sourceModel);
 m_proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
 m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

 m_tableView->setModel(m_proxyModel);
 m_tableView->setSortingEnabled(true);
 m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
 m_tableView->setAlternatingRowColors(true);
 m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
 mainLayout->addWidget(m_tableView);

 QObject::connect(m_btnAdd, &QPushButton::clicked, this, &MainWindow::onAddBook);
 QObject::connect(m_btnSave, &QPushButton::clicked, this, &MainWindow::onSaveData);

 m_stack->addWidget(m_booksPage);
}

void MainWindow::setupReadersPage() {
 m_readersPage = new QWidget();
 QVBoxLayout* mainLayout = new QVBoxLayout(m_readersPage);

 QHBoxLayout* topLayout = new QHBoxLayout();
 m_btnBack = new QPushButton("Назад");
 topLayout->addWidget(m_btnBack);
 QObject::connect(m_btnBack, &QPushButton::clicked, this, &MainWindow::onSwitchToCatalog);

 m_btnRegister = new QPushButton("Регистрация читателя");
 topLayout->addWidget(m_btnRegister);
 QObject::connect(m_btnRegister, &QPushButton::clicked, this, &MainWindow::onRegisterReader);

 m_btnDeleteReader = new QPushButton("Удалить по фамилии");
 topLayout->addWidget(m_btnDeleteReader);
 QObject::connect(m_btnDeleteReader, &QPushButton::clicked, this, &MainWindow::onRemoveReaderBySurname);

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
 m_userTable->setSelectionBehavior(QAbstractItemView::SelectRows);
 tablesLayout->addWidget(m_userTable, 2);

 m_historyTable = new QTableView();
 m_historyModel = new QStandardItemModel(this);
 m_historyModel->setHorizontalHeaderLabels({"ID книги", "Дата выдачи", "Дата возврата", "Статус"});
 m_historyTable->setModel(m_historyModel);
 m_historyTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
 m_historyTable->setAlternatingRowColors(true);
 m_historyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
 tablesLayout->addWidget(m_historyTable, 1);

 mainLayout->addLayout(tablesLayout);

 QObject::connect(m_userTable->selectionModel(), &QItemSelectionModel::currentRowChanged,
                  this, &MainWindow::onReaderSelectionChanged);

 m_stack->addWidget(m_readersPage);
}

// === Навигация ===

void MainWindow::onSwitchToReaders() {
 refreshReadersTable();
 m_stack->setCurrentWidget(m_readersPage);
}

void MainWindow::onSwitchToCatalog() {
 m_stack->setCurrentWidget(m_booksPage);
}

// === Каталог книг ===

void MainWindow::onSearchChanged() {
 QString text = m_searchEdit->text();
 int column = m_searchColumn->currentIndex();
 m_proxyModel->setFilterKeyColumn(column);
 m_proxyModel->setFilterFixedString(text);
}

void MainWindow::onShowChart() {
 const auto& books = m_catalog.getBooks();
 if (books.empty()) {
  QMessageBox::information(this, "Диаграмма", "Каталог пуст — нет данных для диаграммы.");
  return;
 }

 QMap<QString, double> genreCount;
 for (const auto& b : books) {
  QString genre = QString::fromStdString(b.getGenre());
  if (genre.isEmpty()) genre = "Без жанра";
  genreCount[genre] += 1;
 }

 QDialog* dialog = new QDialog(this);
 dialog->setWindowTitle("Диаграмма: Книги по жанрам");
 dialog->resize(600, 500);

 QVBoxLayout* layout = new QVBoxLayout(dialog);
 PieChartWidget* chart = new PieChartWidget(dialog);
 chart->setData(genreCount);
 layout->addWidget(chart);

 QPushButton* btnClose = new QPushButton("Закрыть", dialog);
 QObject::connect(btnClose, &QPushButton::clicked, dialog, &QDialog::close);
 layout->addWidget(btnClose);

 dialog->exec();
 delete dialog;
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
 QObject::connect(btnBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
 QObject::connect(btnBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

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

void MainWindow::onIssueBook() {
 IssueDialog dialog(m_catalog, this);
 if (dialog.exec() == QDialog::Accepted) {
  refreshTable();
  QMessageBox::information(this, "Успех", "Книга успешно выдана читателю.");
 }
}

// === Страница читателей ===

void MainWindow::refreshReadersTable() {
 m_userModel->removeRows(0, m_userModel->rowCount());
 m_historyModel->removeRows(0, m_historyModel->rowCount());
 const auto& users = m_catalog.getUsers();
 for (const auto& u : users) {
  QList<QStandardItem*> row;
  row << new QStandardItem(QString::fromStdString(u.getUserId()))
      << new QStandardItem(QString::fromStdString(u.getFullName()))
      << new QStandardItem(QString::fromStdString(u.getPhoneNumber()));
  m_userModel->appendRow(row);
 }
}

void MainWindow::showReaderHistory(const std::string& userId) {
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

void MainWindow::onRegisterReader() {
 bool ok;
 QString fullName = QInputDialog::getText(this, "Регистрация", "ФИО читателя:", QLineEdit::Normal, "", &ok);
 if (!ok || fullName.trimmed().isEmpty()) return;

 QString phone = QInputDialog::getText(this, "Регистрация", "Номер телефона:", QLineEdit::Normal, "+7", &ok);

 std::string autoId = m_catalog.generateUserId();
 m_catalog.addUser(User(autoId, fullName.trimmed().toStdString(), phone.trimmed().toStdString()));

 refreshReadersTable();
 QMessageBox::information(this, "Успех", "Читатель зарегистрирован.");
}

void MainWindow::onRemoveReaderBySurname() {
 bool ok;
 QString surname = QInputDialog::getText(this, "Удаление читателя",
  "Введите фамилию для удаления:", QLineEdit::Normal, "", &ok);

 if (!ok || surname.trimmed().isEmpty()) return;

 QString searchQuery = surname.trimmed().toLower();
 const auto& users = m_catalog.getUsers();
 auto it = std::find_if(users.begin(), users.end(), [&](const User& u){
  return QString::fromStdString(u.getFullName()).toLower().contains(searchQuery);
 });

 if (it != users.end()) {
  m_catalog.removeUser(it->getUserId());
  refreshReadersTable();
  QMessageBox::information(this, "Успех", "Читатель успешно удалён из системы.");
 } else {
  QMessageBox::information(this, "Информация", "Читателя с такой фамилией в базе нет.");
 }
}

void MainWindow::onReaderSelectionChanged(const QModelIndex& current, const QModelIndex&) {
 if (!current.isValid()) {
  m_historyModel->removeRows(0, m_historyModel->rowCount());
  return;
 }
 QString userId = m_userModel->item(current.row(), 0)->text();
 showReaderHistory(userId.toStdString());
}
