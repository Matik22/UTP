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
#include <QFileDialog>
#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include "issuedialog.h"
#include "chartwidget.h"
#include "user.h"

MainWindow::MainWindow(QWidget* parent)
 : QMainWindow(parent), m_sourceModel(nullptr), m_proxyModel(nullptr),
   m_searchEdit(nullptr), m_searchColumn(nullptr),
   m_userModel(nullptr), m_userProxy(nullptr), m_historyModel(nullptr),
   m_chartWidget(nullptr), m_chartTypeCombo(nullptr), m_chartDataCombo(nullptr) {
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
 setupChartPage();

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

 m_btnImport = new QPushButton("Загрузить из файла");
 topLayout->addWidget(m_btnImport);
 QObject::connect(m_btnImport, &QPushButton::clicked, this, &MainWindow::onImportBooks);

 m_btnIssueBook = new QPushButton("Выдать книгу");
 topLayout->addWidget(m_btnIssueBook);
 QObject::connect(m_btnIssueBook, &QPushButton::clicked, this, &MainWindow::onIssueBook);

    m_btnReturnBook = new QPushButton("Вернуть книгу");
    topLayout->addWidget(m_btnReturnBook);
    QObject::connect(m_btnReturnBook, &QPushButton::clicked, this, &MainWindow::onReturnBook);

 m_btnChart = new QPushButton("Статистика");
 topLayout->addWidget(m_btnChart);
 QObject::connect(m_btnChart, &QPushButton::clicked, this, &MainWindow::onSwitchToChart);

 topLayout->addStretch();
 m_btnSave = new QPushButton("Сохранить");
 topLayout->addWidget(m_btnSave);

 m_btnReport = new QPushButton("Выгрузить отчёт");
 topLayout->addWidget(m_btnReport);
 QObject::connect(m_btnReport, &QPushButton::clicked, this, &MainWindow::onGenerateReport);
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

 // === Поиск читателей ===
 QHBoxLayout* searchLayout = new QHBoxLayout();

 QLabel* lblSearch = new QLabel("Искать по:");
 m_readerSearchColumn = new QComboBox();
 m_readerSearchColumn->addItems({"ФИО", "Телефон", "ID"});

 m_readerSearchEdit = new QLineEdit();
 m_readerSearchEdit->setPlaceholderText("Введите текст для поиска...");

 searchLayout->addWidget(lblSearch);
 searchLayout->addWidget(m_readerSearchColumn);
 searchLayout->addWidget(m_readerSearchEdit);

 mainLayout->addLayout(searchLayout);

 connect(m_readerSearchEdit, &QLineEdit::textChanged,
         this, &MainWindow::onReaderSearchChanged);
 connect(m_readerSearchColumn, QOverload<int>::of(&QComboBox::currentIndexChanged),
         this, &MainWindow::onReaderSearchChanged);

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

 m_btnImportReaders = new QPushButton("Загрузить читателей");
 topLayout->addWidget(m_btnImportReaders);
 connect(m_btnImportReaders, &QPushButton::clicked, this, &MainWindow::onImportReaders);
}

void MainWindow::setupChartPage() {
 m_chartPage = new QWidget();
 QVBoxLayout* mainLayout = new QVBoxLayout(m_chartPage);

 QHBoxLayout* topLayout = new QHBoxLayout();
 m_btnBackFromChart = new QPushButton("Назад");
 topLayout->addWidget(m_btnBackFromChart);
 QObject::connect(m_btnBackFromChart, &QPushButton::clicked, this, &MainWindow::onSwitchToCatalog);

 topLayout->addWidget(new QLabel("Тип диаграммы:"));
 m_chartTypeCombo = new QComboBox();
 m_chartTypeCombo->addItems({"Круговая", "Столбчатая", "Точечная"});
 topLayout->addWidget(m_chartTypeCombo);

 topLayout->addWidget(new QLabel("Данные:"));
 m_chartDataCombo = new QComboBox();
 m_chartDataCombo->addItems({"По жанрам", "В наличии / На руках"});
 topLayout->addWidget(m_chartDataCombo);

 topLayout->addStretch();
 mainLayout->addLayout(topLayout);

 m_chartWidget = new ChartWidget(m_chartPage);
 mainLayout->addWidget(m_chartWidget);

 QObject::connect(m_chartTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                  this, &MainWindow::onChartTypeChanged);
 QObject::connect(m_chartDataCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                  this, &MainWindow::onChartDataChanged);

 m_stack->addWidget(m_chartPage);

 m_btnRegisterLibrarian = new QPushButton("Регистрация библиотекаря");
 topLayout->addWidget(m_btnRegisterLibrarian);
 connect(m_btnRegisterLibrarian, &QPushButton::clicked, this, &MainWindow::onRegisterLibrarian);

}

// === Навигация ===

void MainWindow::onSwitchToReaders() {
 refreshReadersTable();
 m_stack->setCurrentWidget(m_readersPage);
}

void MainWindow::onSwitchToCatalog() {
 m_stack->setCurrentWidget(m_booksPage);
}

void MainWindow::onSwitchToChart() {
 refreshChart();
 m_stack->setCurrentWidget(m_chartPage);
}

// === Каталог книг ===

void MainWindow::onSearchChanged() {
 QString text = m_searchEdit->text();
 int column = m_searchColumn->currentIndex();
 m_proxyModel->setFilterKeyColumn(column);
 m_proxyModel->setFilterFixedString(text);
}

void MainWindow::onImportBooks() {
 QString filename = QFileDialog::getOpenFileName(this, "Загрузить список книг", "",
  "Текстовые файлы (*.txt);;Все файлы (*)");
 if (filename.isEmpty()) return;

 QFile file(filename);
 if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
  QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл.");
  return;
 }

 int added = 0;
 int skipped = 0;
 QTextStream in(&file);

 while (!in.atEnd()) {
  QString line = in.readLine().trimmed();
  if (line.isEmpty() || line.startsWith('#')) continue;

  QStringList parts = line.split("|");
  if (parts.size() < 5) {
   skipped++;
   continue;
  }

  QString title = parts[0].trimmed();
  QString author = parts[1].trimmed();
  QString publisher = parts[2].trimmed();
  bool ok = false;
  int year = parts[3].trimmed().toInt(&ok);
  QString genre = parts[4].trimmed();

  if (title.isEmpty() || author.isEmpty() || !ok) {
   skipped++;
   continue;
  }

  std::string autoId = m_catalog.generateBookId(genre.toStdString());
  Book newBook(autoId, title.toStdString(), author.toStdString(),
               publisher.toStdString(), year, genre.toStdString());
  m_catalog.addBook(newBook);
  added++;
 }

 file.close();
 refreshTable();

 QMessageBox::information(this, "Импорт завершён",
  QString("Добавлено: %1\nПропущено: %2").arg(added).arg(skipped));
}

void MainWindow::refreshTable() {
 m_sourceModel->removeRows(0, m_sourceModel->rowCount());
 const auto& books = m_catalog.getBooks();

 qDebug() << "В каталоге книг:" << books.size();

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
  row[0]->setFlags(row[0]->flags() & ~Qt::ItemIsEditable);
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
 formLayout->addRow(QStringLiteral("Название:"), titleEdit);
 formLayout->addRow(QStringLiteral("Автор:"), authorEdit);
 formLayout->addRow(QStringLiteral("Издательство:"), publisherEdit);
 formLayout->addRow(QStringLiteral("Год издания:"), yearEdit);
 QComboBox* genreCombo = new QComboBox(&dialog);
 genreCombo->addItems({
     "Учебник",
     "Фантастика",
     "Фэнтези",
     "Детектив",
     "Роман",
     "Приключения",
     "Научная литература",
     "История",
     "Поэзия",
     "Детская литература",
     "Документальная",
     "Другое"
 });

 formLayout->addRow(QStringLiteral("Жанр:"), genreCombo);

 QDialogButtonBox* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
 formLayout->addRow(btnBox);
 QObject::connect(btnBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
 QObject::connect(btnBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

 if (dialog.exec() == QDialog::Accepted) {
  QString title = titleEdit->text().trimmed();
  QString author = authorEdit->text().trimmed();
  QString publisher = publisherEdit->text().trimmed();
  int year = yearEdit->value();
  QString genre = genreCombo->currentText();

  if (title.isEmpty() || author.isEmpty()) {
   QMessageBox::warning(this, "Ошибка", "Название и Автор обязательны.");
   return;
  }

  std::string autoId = m_catalog.generateBookId(genre.toStdString());
  Book newBook(autoId, title.toStdString(), author.toStdString(),
               publisher.toStdString(), year, genre.toStdString());

  m_catalog.addBook(newBook);
  m_catalog.autoSave();

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
 m_catalog.autoSave();

}

void MainWindow::onSaveData() {
 m_catalog.saveData();
 QMessageBox::information(this, "Успех", "Данные сохранены");
}

void MainWindow::onIssueBook() {
 IssueDialog dialog(m_catalog, this);
 if (dialog.exec() == QDialog::Accepted) {
  refreshTable();
     m_catalog.autoSave();

  QMessageBox::information(this, "Успех", "Книга успешно выдана читателю.");
 }
}

void MainWindow::onReturnBook() {
    bool ok;
    QString bookId = QInputDialog::getText(this, "Возврат книги", "Введите ID книги:", QLineEdit::Normal, "", &ok);
    if (!ok || bookId.trimmed().isEmpty()) return;
    QString returnDate = QDate::currentDate().toString("yyyy-MM-dd");
    if (m_catalog.returnBook(bookId.trimmed().toStdString(), returnDate.toStdString())) {
        m_catalog.autoSave();
        refreshTable();
        // Обновляем список читателей и их историю, чтобы отразить возврат
        refreshReadersTable();
        QMessageBox::information(this, "Успех", "Книга возвращена.");
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось вернуть книгу (возможно, она не выдана).");
    }
}

// === Страница статистики ===

void MainWindow::onChartTypeChanged() {
 if (!m_chartWidget) return;
 int idx = m_chartTypeCombo->currentIndex();
 m_chartWidget->setChartType(static_cast<ChartWidget::ChartType>(idx));
}

void MainWindow::onChartDataChanged() {
 if (!m_chartWidget) return;
 int idx = m_chartDataCombo->currentIndex();
 m_chartWidget->setDataMode(static_cast<ChartWidget::DataMode>(idx));
 refreshChart();
}

void MainWindow::refreshChart() {
 if (!m_chartWidget) return;

 int dataIdx = m_chartDataCombo ? m_chartDataCombo->currentIndex() : 0;

 if (dataIdx == 0) {
  const auto& books = m_catalog.getBooks();
  QMap<QString, double> genreCount;
  for (const auto& b : books) {
   QString genre = QString::fromStdString(b.getGenre());
   if (genre.isEmpty()) genre = "Без жанра";
   genreCount[genre] += 1;
  }
  m_chartWidget->setData(genreCount);
 } else {
  const auto& books = m_catalog.getBooks();
  double available = 0;
  double issued = 0;
  for (const auto& b : books) {
   if (m_catalog.isBookIssued(b.getBookId())) {
    issued += 1;
   } else {
    available += 1;
   }
  }
  QMap<QString, double> availData;
  availData["В наличии"] = available;
  availData["На руках"] = issued;
  m_chartWidget->setData(availData);
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
  row[0]->setFlags(row[0]->flags() & ~Qt::ItemIsEditable);

  m_userModel->appendRow(row);
 }
}

void MainWindow::showReaderHistory(const std::string& userId) {
 m_historyModel->removeRows(0, m_historyModel->rowCount());
 auto history = m_catalog.getUserHistory(userId);
 for (const auto& rec : history) {
  // Show only currently issued books (returnDate empty)
  if (!rec.getReturnDate().empty()) continue; // skip returned books
  QList<QStandardItem*> row;
  row << new QStandardItem(QString::fromStdString(rec.getBookId()))
      << new QStandardItem(QString::fromStdString(rec.getIssueDate()))
      << new QStandardItem("В обороте") // return date is empty => still in circulation
      << new QStandardItem(rec.getIsOverdue() ? "Просрочка" : "В норме");
  m_historyModel->appendRow(row);
 }
}

void MainWindow::onRegisterReader() {
    QDialog dialog(this);
    dialog.setWindowTitle("Регистрация читателя");
    dialog.setModal(true);
    dialog.setMinimumWidth(350);

    QFormLayout* form = new QFormLayout(&dialog);

    // Поля ввода
    QLineEdit* fullNameEdit = new QLineEdit(&dialog);
    QLineEdit* phoneEdit    = new QLineEdit(&dialog);

    fullNameEdit->setPlaceholderText("Фамилия Имя Отчество");
    phoneEdit->setPlaceholderText("+7XXXXXXXXXX");

    form->addRow("ФИО:", fullNameEdit);
    form->addRow("Телефон:", phoneEdit);

    // Кнопки OK / Cancel
    QDialogButtonBox* btnBox =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form->addRow(btnBox);

    connect(btnBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(btnBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    // Если нажали OK
    if (dialog.exec() == QDialog::Accepted) {

        QString fullName = fullNameEdit->text().trimmed().simplified();
        QString phone    = phoneEdit->text().trimmed();

        // === Валидация ФИО ===
        QRegularExpression badChars("[^А-Яа-яЁё\\-\\s]");
        if (badChars.match(fullName).hasMatch()) {
            QMessageBox::warning(this, "Ошибка",
                                 "ФИО может содержать только русские буквы, пробелы и дефисы");
            return;
        }

        QStringList parts = fullName.split(" ");
        if (parts.size() < 2 || parts.size() > 3) {
            QMessageBox::warning(this, "Ошибка",
                                 "ФИО должно состоять из 2–3 слов");
            return;
        }

        for (const QString& p : parts) {
            if (p.length() < 2 || !p[0].isUpper()) {
                QMessageBox::warning(this, "Ошибка",
                                     "Каждая часть ФИО должна начинаться с заглавной буквы");
                return;
            }
        }

        // === Валидация телефона ===
        if (QRegularExpression("[^0-9\\s\\-\\(\\)\\+]").match(phone).hasMatch()) {
            QMessageBox::warning(this, "Ошибка",
                                 "Телефон может содержать только цифры, +, (), пробелы и тире");
            return;
        }

        QString digitsOnly = phone;
        digitsOnly.remove(QRegularExpression("[^0-9]"));

        if (digitsOnly.length() != 11 || (digitsOnly[0] != '7' && digitsOnly[0] != '8')) {
            QMessageBox::warning(this, "Ошибка",
                                 "Введите корректный номер телефона (11 цифр, начинается с 7 или 8)");
            return;
        }

        QString normalizedPhone = digitsOnly;
        if (normalizedPhone[0] == '7')
            normalizedPhone.prepend("+");

        // === Добавление читателя ===
        std::string autoId = m_catalog.generateUserId();
        m_catalog.addUser(User(autoId,
                               fullName.toStdString(),
                               normalizedPhone.toStdString()));

        m_catalog.autoSave();
        refreshReadersTable();

        QMessageBox::information(this, "Успех", "Читатель зарегистрирован.");
    }
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
     m_catalog.autoSave();

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

void MainWindow::onGenerateReport() {
    QString filePath = QFileDialog::getSaveFileName(
        this,
        "Сохранить отчёт",
        QCoreApplication::applicationDirPath() + "/report.txt",
        "Текстовые файлы (*.txt)"
    );
    if (filePath.isEmpty()) return;

    bool ok = m_catalog.generateReport(filePath.toStdString());
    if (ok) {
        QMessageBox::information(this, "Отчёт сохранён",
            "Отчёт успешно выгружен в файл:\n" + filePath);
        statusBar()->showMessage("Отчёт сохранён: " + filePath);
    } else {
        QMessageBox::warning(this, "Ошибка",
            "Не удалось сохранить отчёт.\nПроверьте путь к файлу.");
    }
}

void MainWindow::onImportReaders() {
    QString filename = QFileDialog::getOpenFileName(
        this,
        "Загрузить список читателей",
        "",
        "Текстовые файлы (*.txt);;Все файлы (*)"
        );
    if (filename.isEmpty()) return;
    // Copy selected file to the standard readers file location and load it
    QString targetPath = QCoreApplication::applicationDirPath() + "/../../../" + LibraryConstants::kReadersFileName;
    // Remove any existing file to ensure fresh copy
    QFile::remove(targetPath);
    if (!QFile::copy(filename, targetPath)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось скопировать файл читателей.");
        return;
    }
    // Load the newly copied readers file
    m_catalog.loadReaders();
    refreshReadersTable();
    QMessageBox::information(this, "Импорт завершён", "Файл читателей загружен.");
    return;

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл.");
        return;
    }

    int added = 0;
    int skipped = 0;
    QTextStream in(&file);

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#')) continue;

        QStringList parts = line.split("|");
        if (parts.size() < 1) { skipped++; continue; }

        QString fullName = parts[0].trimmed();
        QString phone = parts.size() > 1 ? parts[1].trimmed() : "";

        if (fullName.isEmpty()) { skipped++; continue; }

        QString trimmedPhone = phone.trimmed();

        // Проверка на недопустимые символы
        if (QRegularExpression("[^0-9\\s\\-\\(\\)\\+]").match(trimmedPhone).hasMatch()) {
            skipped++;
            continue;
        }

        QString digitsOnly = trimmedPhone;
        digitsOnly.remove(QRegularExpression("[^0-9]"));

        if (!digitsOnly.isEmpty()) {
            if (digitsOnly.length() != 11 || (digitsOnly[0] != '7' && digitsOnly[0] != '8')) {
                skipped++;
                continue;
            }
        }

        QString normalizedPhone = digitsOnly;
        if (!digitsOnly.isEmpty() && normalizedPhone[0] == '7')
            normalizedPhone.prepend("+");

        std::string autoId = m_catalog.generateUserId();
        m_catalog.addUser(User(autoId, fullName.toStdString(), normalizedPhone.toStdString()));

        QString trimmedName = fullName.trimmed().simplified();

        // Проверка символов
        if (QRegularExpression("[^А-Яа-яЁё\\-\\s]").match(trimmedName).hasMatch()) {
            skipped++;
            continue;
        }

        QStringList nameParts = trimmedName.split(" ");
        if (nameParts.size() < 2 || nameParts.size() > 3) {
            skipped++;
            continue;
        }

        bool bad = false;
        for (const QString& p : std::as_const(nameParts)){
            if (p.length() < 2 || !p[0].isUpper()) {
                bad = true;
                break;
            }
        }
        if (bad) {
            skipped++;
            continue;
        }

        QString normalizedName = trimmedName;

        added++;
    }

    file.close();
    refreshReadersTable();

    QMessageBox::information(
        this,
        "Импорт завершён",
        QString("Добавлено: %1\nПропущено: %2").arg(added).arg(skipped)
        );
}

void MainWindow::onReaderSearchChanged() {
    QString text = m_readerSearchEdit->text();
    int column = m_readerSearchColumn->currentIndex();

    int realColumn = 0;
    if (column == 0) realColumn = 1;   // ФИО
    if (column == 1) realColumn = 2;   // Телефон
    if (column == 2) realColumn = 0;   // ID

    m_userProxy->setFilterKeyColumn(realColumn);
    m_userProxy->setFilterFixedString(text);
}

// Регистрация
void MainWindow::onRegisterLibrarian() {
    bool ok;
    QString login = QInputDialog::getText(this, "Регистрация", "Логин:", QLineEdit::Normal, "", &ok);
    if (!ok || login.isEmpty()) return;

    QString pass = QInputDialog::getText(this, "Регистрация", "Пароль:", QLineEdit::Normal, "", &ok);
    if (!ok || pass.isEmpty()) return;

    m_catalog.addLibrarian(Librarian(login.toStdString(), pass.toStdString()));
    m_catalog.autoSave();

    QMessageBox::information(this, "Успех", "Библиотекарь зарегистрирован.");
}
