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
    QFile file(QString::fromStdString(filePath));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    // Используем QTextStream с UTF-8
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out.setGenerateByteOrderMark(true);

    QString now = QDate::currentDate().toString("dd.MM.yyyy");

    // Вспомогательная функция для безопасного преобразования строк
    auto toQString = [](const std::string& str) -> QString {
        // Пробуем разные кодировки
        QString result = QString::fromUtf8(str.c_str());
        if (result.contains(QChar::ReplacementCharacter)) {
            // Если есть символы замены, пробуем Windows-1251
            result = QString::fromLatin1(str.c_str());
        }
        return result;
    };

    out << "============================================================\n";
    out << "       ОТЧЁТ ПО БИБЛИОТЕЧНОМУ ФОНДУ\n";
    out << "       Дата формирования: " << now << "\n";
    out << "============================================================\n\n";

    // 1. Общая статистика
    int totalBooks = static_cast<int>(m_books.size());
    int issuedCount = 0;
    for (const auto& r : m_issueRecords) {
        if (r.getReturnDate().empty()) ++issuedCount;
    }
    int availableCount = totalBooks - issuedCount;

    out << "[ ОБЩАЯ СТАТИСТИКА ]\n";
    out << "  Всего изданий в фонде : " << totalBooks << "\n";
    out << "  Выдано сейчас         : " << issuedCount << "\n";
    out << "  Доступно              : " << availableCount << "\n";
    out << "  Зарегистрировано чит. : " << m_users.size() << "\n";
    out << "  Всего операций выдачи : " << m_issueRecords.size() << "\n\n";

    // 2. Каталог изданий
    out << "------------------------------------------------------------\n";
    out << "[ КАТАЛОГ ИЗДАНИЙ ]\n";
    out << "------------------------------------------------------------\n";

    // Форматирование с помощью QString
    for (const auto& b : m_books) {
        bool issued = isBookIssued(b.getBookId());
        QString id = toQString(b.getBookId());
        QString title = toQString(b.getTitle());
        QString author = toQString(b.getAuthor());
        QString genre = toQString(b.getGenre());

        // Обрезаем длинные строки
        if (title.length() > 30) title = title.left(29) + "…";
        if (author.length() > 20) author = author.left(19) + "…";

        // Форматируем вывод
        out << qSetFieldWidth(8) << id
            << " " << qSetFieldWidth(30) << title
            << " " << qSetFieldWidth(20) << author
            << " " << qSetFieldWidth(6) << b.getYear()
            << " " << qSetFieldWidth(15) << genre
            << qSetFieldWidth(0) << " " << (issued ? "ВЫДАНА" : "ДОСТУПНА") << "\n";
    }
    out << "\n";

    // 3. Текущие выдачи
    out << "------------------------------------------------------------\n";
    out << "[ ТЕКУЩИЕ ВЫДАЧИ ]\n";
    out << "------------------------------------------------------------\n";
    bool hasActive = false;
    for (const auto& r : m_issueRecords) {
        if (!r.getReturnDate().empty()) continue;
        hasActive = true;

        // Найти книгу
        QString bookTitle = toQString(r.getBookId());
        for (const auto& b : m_books) {
            if (b.getBookId() == r.getBookId()) {
                bookTitle = toQString(b.getTitle());
                break;
            }
        }

        // Найти пользователя
        QString userName = toQString(r.getUserId());
        for (const auto& u : m_users) {
            if (u.getUserId() == r.getUserId()) {
                userName = toQString(u.getFullName());
                break;
            }
        }

        out << "  Книга    : " << bookTitle << " [" << toQString(r.getBookId()) << "]\n";
        out << "  Читатель : " << userName << " [" << toQString(r.getUserId()) << "]\n";
        out << "  Выдана   : " << toQString(r.getIssueDate()) << "\n";
        QString returnDateStr = r.getReturnDate().empty() ? "не возвращена" : toQString(r.getReturnDate());
        out << "  Вернуть  : " << returnDateStr << "\n";
        out << "  " << QString(40, '-') << "\n";
    }
    if (!hasActive) out << "  Нет активных выдач.\n";
    out << "\n";

    // 4. История возвратов
    out << "------------------------------------------------------------\n";
    out << "[ ИСТОРИЯ ВОЗВРАТОВ ]\n";
    out << "------------------------------------------------------------\n";
    bool hasHistory = false;
    for (const auto& r : m_issueRecords) {
        if (r.getReturnDate().empty()) continue;
        hasHistory = true;

        QString bookTitle = toQString(r.getBookId());
        for (const auto& b : m_books) {
            if (b.getBookId() == r.getBookId()) {
                bookTitle = toQString(b.getTitle());
                break;
            }
        }

        QString userName = toQString(r.getUserId());
        for (const auto& u : m_users) {
            if (u.getUserId() == r.getUserId()) {
                userName = toQString(u.getFullName());
                break;
            }
        }

        out << "  " << bookTitle << " | " << userName
            << " | Выд: " << toQString(r.getIssueDate())
            << " | Возвр: " << toQString(r.getReturnDate()) << "\n";
    }
    if (!hasHistory) out << "  История возвратов пуста.\n";
    out << "\n";

    out << "============================================================\n";
    out << "  Конец отчёта\n";
    out << "============================================================\n";

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