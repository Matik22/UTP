#include "catalog.h"
#include <fstream>
#include <iostream>
#include <QCoreApplication>
#include <QDebug>
#include <QDate>
#include <QMap>
#include <QDir>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <QtCore/qfileinfo.h>
#include <QFile>
#include <QTextStream>
#include <QString>

Catalog::Catalog() {
    loadLibrarians();  // Загружаем библиотекарей при создании
}

QString Catalog::getDataFilePath() const {
    // 1. Рядом с исполняемым файлом
    QString exeDir = QCoreApplication::applicationDirPath();
    QString path = exeDir + "/" + LibraryConstants::kDataFileName;
    if (QFileInfo::exists(path)) return path;

    // 2. Текущая рабочая директория (актуально при запуске из Qt Creator на Linux)
    QString cwdPath = QDir::currentPath() + "/" + LibraryConstants::kDataFileName;
    if (QFileInfo::exists(cwdPath)) return cwdPath;

    // 3. Файл не найден — вернём путь рядом с exe (туда и сохраним при первом Save)
    return path;
}

QString Catalog::getLibrariansFilePath() const {
    // 1. Рядом с исполняемым файлом
    QString exeDir = QCoreApplication::applicationDirPath();
    QString path = exeDir + "/" + LibraryConstants::kLibrariansFileName;
    if (QFileInfo::exists(path)) return path;

    // 2. Текущая рабочая директория
    QString cwdPath = QDir::currentPath() + "/" + LibraryConstants::kLibrariansFileName;
    if (QFileInfo::exists(cwdPath)) return cwdPath;

    // 3. Файл не найден — вернём путь рядом с exe (туда и сохраним)
    return path;
}

bool Catalog::removeUser(const std::string& userId) {
    auto it = std::remove_if(m_users.begin(), m_users.end(),
                             [&](const User& u){ return u.getUserId() == userId; });
    if (it != m_users.end()) {
        m_users.erase(it, m_users.end());
        return true;
    }
    return false;
}

bool Catalog::removeBook(const std::string& bookId) {
    bool isIssued = std::any_of(m_issueRecords.begin(), m_issueRecords.end(),
                                [&](const IssueRecord& r){ return r.getBookId() == bookId && r.getReturnDate().empty(); });
    if (isIssued) return false;

    auto it = std::remove_if(m_books.begin(), m_books.end(),
                             [&](const Book& b){ return b.getBookId() == bookId; });
    if (it != m_books.end()) {
        m_books.erase(it, m_books.end());
        return true;
    }
    return false;
}


std::string Catalog::generateUserId() const {
    int maxNum = 0;
    for (const auto& u : m_users) {
        const auto& id = u.getUserId();
        if (id.size() > 1 && id[0] == 'R') {
            try {
                int num = std::stoi(id.substr(1));
                if (num > maxNum) maxNum = num;
            } catch (...) {}
        }
    }
    std::ostringstream oss;
    oss << 'R' << std::setw(3) << std::setfill('0') << (maxNum + 1);
    return oss.str();
}

std::string Catalog::generateBookId(const std::string& genre) const {
    // Используем первую латинскую букву транслитерации жанра как префикс.
    // genre может быть UTF-8 строкой с кириллицей — genre[0] даст неверный байт,
    // поэтому берём первый Unicode-символ через QString и транслитерируем его в ASCII.
    QString qGenre = QString::fromStdString(genre);
    QChar firstChar = qGenre.isEmpty() ? QChar('X') : qGenre[0].toUpper();

    // Маппинг первой буквы кириллицы → латинский префикс
    static const QMap<QChar, char> cyrMap = {
                                             {QChar(0x0410),'A'}, {QChar(0x0411),'B'}, {QChar(0x0412),'V'},
                                             {QChar(0x0413),'G'}, {QChar(0x0414),'D'}, {QChar(0x0415),'E'},
                                             {QChar(0x0416),'Z'}, {QChar(0x0417),'Z'}, {QChar(0x0418),'I'},
                                             {QChar(0x0419),'Y'}, {QChar(0x041A),'K'}, {QChar(0x041B),'L'},
                                             {QChar(0x041C),'M'}, {QChar(0x041D),'N'}, {QChar(0x041E),'O'},
                                             {QChar(0x041F),'P'}, {QChar(0x0420),'R'}, {QChar(0x0421),'S'},
                                             {QChar(0x0422),'T'}, {QChar(0x0423),'U'}, {QChar(0x0424),'F'},
                                             {QChar(0x0425),'H'}, {QChar(0x0426),'C'}, {QChar(0x0427),'C'},
                                             {QChar(0x0428),'S'}, {QChar(0x0429),'S'}, {QChar(0x042A),'X'},
                                             {QChar(0x042B),'Y'}, {QChar(0x042C),'X'}, {QChar(0x042D),'E'},
                                             {QChar(0x042E),'Y'}, {QChar(0x042F),'Y'},
                                             };

    char prefix;
    if (firstChar.isLetter() && firstChar.unicode() < 128) {
        prefix = static_cast<char>(firstChar.toUpper().toLatin1());
    } else {
        prefix = cyrMap.value(firstChar, 'X');
    }

    int maxNum = 0;
    for (const auto& b : m_books) {
        const auto& id = b.getBookId();
        if (!id.empty() && id[0] == prefix) {
            try {
                int num = std::stoi(id.substr(1));
                if (num > maxNum) maxNum = num;
            } catch (...) {}
        }
    }
    std::ostringstream oss;
    oss << prefix << std::setw(3) << std::setfill('0') << (maxNum + 1);
    return oss.str();
}
void Catalog::saveData() const {
    QString filePath = getDataFilePath();
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Ошибка открытия файла для сохранения:" << filePath;
        return;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");
    out.setGenerateByteOrderMark(true);

    // 1. Секция книг
    out << "===BOOKS===\n";
    for (const auto& b : m_books) {
        out << QString::fromStdString(b.toFileString()) << "\n";
    }

    // 2. Секция читателей
    out << "\n===USERS===\n";
    for (const auto& u : m_users) {
        out << QString::fromStdString(u.toFileString()) << "\n";
    }

    // 3. Секция выдач/возвратов
    out << "\n===ISSUES===\n";
    for (const auto& i : m_issueRecords) {
        out << QString::fromStdString(i.toFileString()) << "\n";
    }

    file.close();
    qDebug() << "[OK] Данные сохранены в:" << filePath;
}

void Catalog::loadData() {
    QString filePath = getDataFilePath();
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Файл данных не найден:" << filePath;
        qDebug() << "Приложение запущено с пустым каталогом";
        return;
    }

    QTextStream in(&file);
    in.setCodec("UTF-8");

    std::string line, section;
    int loadedBooks = 0, loadedUsers = 0, loadedIssues = 0;

    while (!in.atEnd()) {
        QString qline = in.readLine();
        std::string line = qline.toStdString();

        // Убираем BOM если есть
        if (!line.empty() && (unsigned char)line[0] == 0xEF &&
            (unsigned char)line[1] == 0xBB && (unsigned char)line[2] == 0xBF) {
            line = line.substr(3);
        }

        // Убираем \r
        if (!line.empty() && line.back() == '\r') line.pop_back();

        if (line.empty() || line[0] == '#') continue;
        if (line.find("===BOOKS===") != std::string::npos) { section = "books"; continue; }
        if (line.find("===USERS===") != std::string::npos) { section = "users"; continue; }
        if (line.find("===ISSUES===") != std::string::npos) { section = "issues"; continue; }

        try {
            if (section == "books" && m_books.size() < LibraryConstants::kMaxBooks) {
                m_books.push_back(Book::fromFileString(line));
                loadedBooks++;
            } else if (section == "users" && m_users.size() < LibraryConstants::kMaxUsers) {
                m_users.push_back(User::fromFileString(line));
                loadedUsers++;
            } else if (section == "issues") {
                m_issueRecords.push_back(IssueRecord::fromFileString(line));
                loadedIssues++;
            }
        } catch (const std::exception& e) {
            qDebug() << "Ошибка парсинга строки:" << line.c_str();
        }
    }
    file.close();
    qDebug() << "Загрузка завершена. Книг:" << loadedBooks << "| Читателей:" << loadedUsers << "| Выдач:" << loadedIssues;
}

const std::vector<Book>& Catalog::getBooks() const { return m_books; }


void Catalog::addBook(const Book& book) {
    if (m_books.size() < LibraryConstants::kMaxBooks) {
        m_books.push_back(book);
    }
}

void Catalog::updateBook(const Book& updatedBook){
    for (auto& book : m_books) {
        if (book.getBookId() == updatedBook.getBookId()) {
            book = updatedBook;
            return;
        }
    }
}

void Catalog::updateUser(const User& updatedUser) {
    for(auto& user : m_users){
        if(user.getUserId() == updatedUser.getUserId()){
            user = updatedUser;
            return;
        }
    }
}

void Catalog::addUser(const User& user) {
    if (m_users.size() < LibraryConstants::kMaxUsers) {
        m_users.push_back(user);
    }
}

const std::vector<User>& Catalog::getUsers() const { return m_users; }

std::vector<IssueRecord> Catalog::getUserHistory(const std::string& userId) const {
    std::vector<IssueRecord> history;
    for (const auto& rec : m_issueRecords) {
        if (rec.getUserId() == userId) history.push_back(rec);
    }
    return history;
}

bool Catalog::isBookIssued(const std::string& bookId) const {
    return std::any_of(m_issueRecords.begin(), m_issueRecords.end(),
                       [&](const IssueRecord& r){ return r.getBookId() == bookId && r.getReturnDate().empty(); });
}

std::vector<Book> Catalog::getAvailableBooks() const {
    std::vector<Book> available;
    for (const auto& b : m_books) {
        if (!isBookIssued(b.getBookId())) available.push_back(b);
    }
    return available;
}

bool Catalog::issueBook(const std::string& bookId, const std::string& userId,
                        const std::string& issueDate, const std::string& returnDate) {
    auto bookIt = std::find_if(m_books.begin(), m_books.end(),
                               [&](const Book& b){ return b.getBookId() == bookId; });
    if (bookIt == m_books.end()) return false;

    auto userIt = std::find_if(m_users.begin(), m_users.end(),
                               [&](const User& u){ return u.getUserId() == userId; });
    if (userIt == m_users.end()) return false;

    if (isBookIssued(bookId)) return false;

    m_issueRecords.emplace_back(bookId, userId, issueDate, returnDate, false);
    return true;
}

bool Catalog::returnBook(const std::string& bookId, const std::string& returnDate) {
    // Find the active issue record (book issued and not yet returned)
    auto it = std::find_if(m_issueRecords.begin(), m_issueRecords.end(),
                           [&](const IssueRecord& r){ return r.getBookId() == bookId && r.getReturnDate().empty(); });
    if (it == m_issueRecords.end()) return false;
    // Update the record with the return date and clear overdue flag
    it->setReturnDate(returnDate);
    it->setIsOverdue(false);
    return true;
}

const std::vector<IssueRecord>& Catalog::getIssueRecords() const { return m_issueRecords; }

bool Catalog::generateReport(const std::string& filePath) const {
    // Переводим std::string пути в родной для Qt QString
    QString path = QString::fromStdString(filePath);

    // Если путь пустой (например, пользователь отменил диалог), выходим
    if (path.isEmpty()) return false;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    // Для Qt 5 используем setCodec, чтобы не было кракозябр
    out.setCodec("UTF-8");

    QDate today = QDate::currentDate();

    /* ---------- сбор статистики ---------- */
    int totalBooks = m_books.size();
    int activeIssues = 0;
    int overdueCount = 0;

    // Посчитаем количество активных выдач на текущий момент (где книга еще не возвращена)
    // Предполагаем, что у пустой записи возврата пустая строка "" или специальный маркер
    for (const auto& record : m_issueRecords) {
        if (record.getReturnDate().empty()) {
            activeIssues++;
            if (record.getIsOverdue()) {
                overdueCount++;
            }
        }
    }
    int availableBooks = totalBooks - activeIssues;

    // Распределение книг по жанрам (для статистики)
    QMap<QString, int> genreCount;
    // ТОП-5 самых активных читателей (по количеству взятых когда-либо книг)
    QMap<QString, int> readerActivity;

    for (const auto& book : m_books) {
        genreCount[QString::fromStdString(book.getGenre())]++;
    }

    for (const auto& record : m_issueRecords) {
        readerActivity[QString::fromStdString(record.getUserId())]++;
    }

    /* ---------- вывод отчёта ---------- */
    out << "═══════════════════════════════════════════════════════\n";
    out << "         АНАЛИТИЧЕСКИЙ ОТЧЁТ ПО БИБЛИОТЕКЕ\n";
    out << "═══════════════════════════════════════════════════════\n";
    out << "Дата формирования: " << today.toString("dd.MM.yyyy") << "\n";
    out << "Всего книг в фонде: " << totalBooks << "\n\n";

    out << "─── ОБЩАЯ СТАТИСТИКА ─────────────────────────────────\n";
    out << QString("Доступно книг:      %1 шт.\n").arg(availableBooks);
    out << QString("Выдано на руки:     %1 шт.\n").arg(activeIssues);
    out << QString("Из них в просрочке: %1 шт.\n").arg(overdueCount);
    out << QString("Зарегистрировано читателей: %1\n\n").arg(m_users.size());

    out << "─── КАТАЛОГ ИЗДАНИЙ ──────────────────────────────────\n";
    out << "ID     | Название книги               | Автор               | Год  | Жанр           | Статус\n";
    out << "────────────────────────────────────────────────────────────────────────────────────────────\n";

    for (const auto& book : m_books) {
        QString bookId = QString::fromStdString(book.getBookId());
        QString title = QString::fromStdString(book.getTitle());
        QString author = QString::fromStdString(book.getAuthor());
        QString genre = QString::fromStdString(book.getGenre());

        // Проверяем статус через вашу существующую функцию
        bool issued = isBookIssued(book.getBookId());
        QString status = issued ? "ВЫДАНА" : "ДОСТУПНА";

        // Выравнивание строк средствами Qt (.arg с отрицательной шириной)
        out << QString("%1 | %2 | %3 | %4 | %5 | %6\n")
                   .arg(bookId, -6)
                   .arg(title.left(28), -28)   // Ограничиваем длину, чтобы таблица не разъезжалась
                   .arg(author.left(19), -19)
                   .arg(book.getYear(), -4)
                   .arg(genre.left(14), -14)
                   .arg(status);
    }
    out << "\n";

    out << "─── РАСПРЕДЕЛЕНИЕ ФОНДА ПО ЖАНРАМ ────────────────────\n";
    for (auto it = genreCount.begin(); it != genreCount.end(); ++it) {
        double percentage = totalBooks > 0 ? (double(it.value()) / totalBooks) * 100.0 : 0.0;
        out << QString("%1: %2 шт. (%3%)\n")
                   .arg(it.key(), -20)
                   .arg(it.value(), 4)
                   .arg(QString::number(percentage, 'f', 1));
    }
    out << "\n";

    out << "─── ТОП ЧИТАТЕЛЕЙ ПО АКТИВНОСТИ ──────────────────────\n";
    QList<QPair<QString, int>> topReaders;
    for (auto it = readerActivity.begin(); it != readerActivity.end(); ++it) {
        topReaders.append({it.key(), it.value()});
    }
    // Сортировка по убыванию активности
    std::sort(topReaders.begin(), topReaders.end(),
              [](const auto &a, const auto &b){ return a.second > b.second; });

    for (int i = 0; i < qMin(5, topReaders.size()); ++i) {
        // Ищем имя пользователя по его ID
        QString userName = "Неизвестный читатель";
        for (const auto& user : m_users) {
            if (user.getUserId() == topReaders[i].first.toStdString()) {
                // ТАК НАДО: используем getFullName() вместо getName()
                userName = QString::fromStdString(user.getFullName());
                break;
            }
        }
        out << QString("%1. ID: %2 | %3 | Взято книг: %4\n")
                   .arg(i + 1)
                   .arg(topReaders[i].first, -5)
                   .arg(userName.left(30), -30) // Ограничим длину имени для ровной таблицы
                   .arg(topReaders[i].second, 3);
    }

    out << "\n═══════════════════════════════════════════════════════\n";
    out << "                    Конец отчёта\n";
    out << "═══════════════════════════════════════════════════════\n";

    file.close();
    return true;
}

void Catalog::loadLibrarians() {
    m_librarians.clear();  // Очищаем перед загрузкой

    QString path = getLibrariansFilePath();

    if (!QFileInfo::exists(path)) {
        qDebug() << "Файл библиотекарей не найден:" << path;
        qDebug() << "Будет создан новый файл при сохранении";
        return;
    }

    std::ifstream file(path.toStdString());
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty()) {
                try {
                    Librarian lib = Librarian::fromFileString(line);
                    // Проверка на дубликаты
                    bool exists = false;
                    for (const auto& existing : m_librarians) {
                        if (existing.login() == lib.login()) {
                            exists = true;
                            break;
                        }
                    }
                    if (!exists) {
                        m_librarians.push_back(lib);
                        qDebug() << "Загружен библиотекарь:" << lib.login().c_str();
                    }
                } catch (const std::exception& e) {
                    qDebug() << "Ошибка парсинга строки:" << line.c_str();
                }
            }
        }
        file.close();
    }

    qDebug() << "Всего загружено библиотекарей:" << m_librarians.size();

    // Если библиотекарей нет, создаем администратора по умолчанию
    if (m_librarians.empty()) {
        qDebug() << "Создание библиотекаря по умолчанию (admin/admin)";
        Librarian defaultAdmin("admin", "admin");
        m_librarians.push_back(defaultAdmin);
        saveLibrarians();  // Сохраняем сразу
    }
}

void Catalog::saveLibrarians() const {
    QString path = getLibrariansFilePath();

    qDebug() << "Сохранение библиотекарей в:" << path;
    qDebug() << "Количество сохраняемых библиотекарей:" << m_librarians.size();

    std::ofstream file(path.toStdString(), std::ios::trunc);
    if (file.is_open()) {
        for (const auto& l : m_librarians) {
            file << l.toFileString() << "\n";
            qDebug() << "Сохранён библиотекарь:" << l.login().c_str();
        }
        file.close();
        qDebug() << "Сохранение завершено успешно";
    } else {
        qDebug() << "ОШИБКА: Не удалось открыть файл для сохранения:" << path;
    }
}

bool Catalog::checkLibrarian(const std::string& login, const std::string& password) const {
    qDebug() << "Проверка библиотекаря - логин:" << login.c_str();
    qDebug() << "Всего библиотекарей в системе:" << m_librarians.size();

    for (const auto& l : m_librarians) {
        qDebug() << "Проверяю с:" << l.login().c_str();
        if (l.login() == login && l.password() == password) {
            qDebug() << "УСПЕШНО! Найден библиотекарь:" << login.c_str();
            return true;
        }
    }
    qDebug() << "НЕ УСПЕШНО! Библиотекарь не найден:" << login.c_str();
    return false;
}

void Catalog::addLibrarian(const Librarian& lib) {
    // Проверяем, существует ли уже такой логин
    for (const auto& existing : m_librarians) {
        if (existing.login() == lib.login()) {
            qDebug() << "Библиотекарь с логином" << lib.login().c_str() << "уже существует!";
            return;
        }
    }

    // Добавляем в список
    m_librarians.push_back(lib);
    qDebug() << "Добавлен новый библиотекарь:" << lib.login().c_str();

    // Сохраняем обновленный список
    saveLibrarians();
}

void Catalog::autoSave() {
    saveData();
    saveLibrarians();
    // also persist separate readers and issues files
    saveReaders();
    saveIssues();
}

// Load readers from dedicated file
void Catalog::loadReaders() {
    // Clear existing list to avoid duplicates
    m_users.clear();
    // Try to load readers from the standard file; if it does not exist, fall back to the test file.
    std::string basePath = QCoreApplication::applicationDirPath().toStdString() + "/../../../";
    std::string fullPath = basePath + LibraryConstants::kReadersFileName;
    std::ifstream file(fullPath);
    if (!file.is_open()) {
        // fallback to test_readers.txt in the same directory
        std::string testPath = basePath + "test_readers.txt";
        file.open(testPath);
        if (!file.is_open()) return; // nothing to load
    }
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty())
            m_users.push_back(User::fromFileString(line));
    }
}

// Save readers to dedicated file
void Catalog::saveUser() const {
    // Alias for saving readers, kept for backward compatibility
    saveReaders();
}

void Catalog::saveReaders() const {
    std::string basePath = QCoreApplication::applicationDirPath().toStdString() + "/../../../";
    std::ofstream file(basePath + LibraryConstants::kReadersFileName, std::ios::trunc);
    for (const auto& u : m_users)
        file << u.toFileString() << "\n";
}

// Load issue records from dedicated file
void Catalog::loadIssueRecords() {
    // Ensure the issues file exists; if it doesn't, create an empty one
    std::string basePath = QCoreApplication::applicationDirPath().toStdString() + "/../../../";
    std::string fullPath = basePath + LibraryConstants::kIssuesFileName;
    std::ifstream file(fullPath);
    if (!file.is_open()) {
        // create empty file
        std::ofstream create(fullPath);
        create.close();
        file.open(fullPath);
        if (!file.is_open()) return; // give up if still cannot open
    }
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty())
            m_issueRecords.push_back(IssueRecord::fromFileString(line));
    }
}

// Save issue records to dedicated file
void Catalog::saveIssues() const {
    std::string basePath = QCoreApplication::applicationDirPath().toStdString() + "/../../../";
    std::ofstream file(basePath + LibraryConstants::kIssuesFileName, std::ios::trunc);
    for (const auto& i : m_issueRecords)
        file << i.toFileString() << "\n";
}

// Simple automatic issuing: give each book to the first reader if not already issued
void Catalog::autoIssueAllBooksToReaders() {
    if (m_users.empty()) return;
    const std::string firstUserId = m_users.front().getUserId();
    QString today = QDate::currentDate().toString("yyyy-MM-dd");
    for (const auto& b : m_books) {
        if (isBookIssued(b.getBookId())) continue;
        // create issue record with empty return date
        IssueRecord rec(b.getBookId(), firstUserId, today.toStdString(), "", false);
        m_issueRecords.push_back(rec);
    }
    // persist changes
    saveIssues();
    saveData();
}

void Catalog::debugPrintLibrarians() const {
    qDebug() << "=================== БИБЛИОТЕКАРИ ===================";
    qDebug() << "Всего:" << m_librarians.size();
    for (const auto& l : m_librarians) {
        qDebug() << "Логин:" << l.login().c_str() << "| Пароль:" << l.password().c_str();
    }
    qDebug() << "====================================================";

    // Выводим путь к файлу
    QString path = getLibrariansFilePath();
    qDebug() << "Путь к файлу:" << path;

    // Проверяем содержимое файла
    std::ifstream file(path.toStdString());
    if (file.is_open()) {
        qDebug() << "Содержимое файла:";
        std::string line;
        while (std::getline(file, line)) {
            qDebug() << line.c_str();
        }
        file.close();
    } else {
        qDebug() << "Файл не найден или не открывается!";
    }
}