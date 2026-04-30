#include "catalog.h"
#include <fstream>
#include <iostream>
#include <QCoreApplication>
#include <QDebug>
#include <QDate>
#include <QMap>
#include <QDir>
#include <QFileInfo>
#include <sstream>
#include <iomanip>
#include <algorithm>

Catalog::Catalog() = default;

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

void Catalog::loadData() {
    QString filePath = getDataFilePath();
    std::ifstream file(filePath.toStdString());

    if (!file.is_open()) {
        qDebug() << "[WARN] Файл данных не найден:" << filePath;
        qDebug() << "[INFO] Приложение запущено с пустым каталогом.";
        return;
    }

    std::string line, section;
    int loadedBooks = 0, loadedUsers = 0, loadedIssues = 0;

    while (std::getline(file, line)) {
        // Убираем \r от Windows-переносов строк
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
            qDebug() << "[WARN] Ошибка парсинга [" << QString::fromStdString(section)
                << "]:" << QString::fromStdString(line) << "→" << e.what();
        }
    }
    qDebug() << "[OK] Загрузка завершена. Книг:" << loadedBooks
             << "| Читателей:" << loadedUsers << "| Выдач:" << loadedIssues;
}

void Catalog::saveData() const {
    QString filePath = getDataFilePath();
    std::ofstream file(filePath.toStdString(), std::ios::trunc);

    if (!file.is_open()) {
        qDebug() << "[ERROR] Не удалось открыть файл для сохранения:" << filePath;
        return;
    }

    // 1. Секция книг
    file << "===BOOKS===\n";
    for (const auto& b : m_books) file << b.toFileString() << "\n";

    // 2. Секция читателей
    file << "\n===USERS===\n";
    for (const auto& u : m_users) file << u.toFileString() << "\n";

    // 3. Секция выдач/возвратов
    file << "\n===ISSUES===\n";
    for (const auto& i : m_issueRecords) file << i.toFileString() << "\n";

    qDebug() << "[OK] Данные сохранены в:" << filePath
             << "| Книг:" << m_books.size() << "| Читателей:" << m_users.size()
             << "| Выдач:" << m_issueRecords.size();
}

const std::vector<Book>& Catalog::getBooks() const { return m_books; }


void Catalog::addBook(const Book& book) {
    if (m_books.size() < LibraryConstants::kMaxBooks) {
        m_books.push_back(book);
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
    auto it = std::find_if(m_issueRecords.begin(), m_issueRecords.end(),
                           [&](const IssueRecord& r){ return r.getBookId() == bookId && r.getReturnDate().empty(); });
    if (it == m_issueRecords.end()) return false;
    it->setReturnDate(returnDate);
    it->setIsOverdue(false);
    return true;
}

const std::vector<IssueRecord>& Catalog::getIssueRecords() const { return m_issueRecords; }
bool Catalog::generateReport(const std::string& filePath) const {
    std::ofstream file(filePath);
    if (!file.is_open()) return false;

    auto now = QDate::currentDate().toString("dd.MM.yyyy").toStdString();

    file << "============================================================\n";
    file << "       ОТЧЁТ ПО БИБЛИОТЕЧНОМУ ФОНДУ\n";
    file << "       Дата формирования: " << now << "\n";
    file << "============================================================\n\n";

    // 1. Общая статистика
    int totalBooks = static_cast<int>(m_books.size());
    int issuedCount = 0;
    for (const auto& r : m_issueRecords)
        if (r.getReturnDate().empty()) ++issuedCount;
    int availableCount = totalBooks - issuedCount;

    file << "[ ОБЩАЯ СТАТИСТИКА ]\n";
    file << "  Всего изданий в фонде : " << totalBooks << "\n";
    file << "  Выдано сейчас         : " << issuedCount << "\n";
    file << "  Доступно              : " << availableCount << "\n";
    file << "  Зарегистрировано чит. : " << m_users.size() << "\n";
    file << "  Всего операций выдачи : " << m_issueRecords.size() << "\n\n";

    // 2. Каталог изданий
    file << "------------------------------------------------------------\n";
    file << "[ КАТАЛОГ ИЗДАНИЙ ]\n";
    file << "------------------------------------------------------------\n";
    file << std::left
         << std::setw(8)  << "ID"
         << std::setw(32) << "Название"
         << std::setw(22) << "Автор"
         << std::setw(8)  << "Год"
         << std::setw(16) << "Жанр"
         << "Статус\n";
    file << std::string(96, '-') << "\n";
    for (const auto& b : m_books) {
        bool issued = isBookIssued(b.getBookId());
        std::string title = b.getTitle().size() > 30 ? b.getTitle().substr(0, 29) + "…" : b.getTitle();
        std::string author = b.getAuthor().size() > 20 ? b.getAuthor().substr(0, 19) + "…" : b.getAuthor();
        file << std::left
             << std::setw(8)  << b.getBookId()
             << std::setw(32) << title
             << std::setw(22) << author
             << std::setw(8)  << b.getYear()
             << std::setw(16) << b.getGenre()
             << (issued ? "ВЫДАНА" : "ДОСТУПНА") << "\n";
    }
    file << "\n";

    // 3. Текущие выдачи
    file << "------------------------------------------------------------\n";
    file << "[ ТЕКУЩИЕ ВЫДАЧИ ]\n";
    file << "------------------------------------------------------------\n";
    bool hasActive = false;
    for (const auto& r : m_issueRecords) {
        if (!r.getReturnDate().empty()) continue;
        hasActive = true;
        // найти книгу и пользователя
        std::string bookTitle = r.getBookId();
        for (const auto& b : m_books)
            if (b.getBookId() == r.getBookId()) { bookTitle = b.getTitle(); break; }
        std::string userName = r.getUserId();
        for (const auto& u : m_users)
            if (u.getUserId() == r.getUserId()) { userName = u.getFullName(); break; }

        file << "  Книга    : " << bookTitle << " [" << r.getBookId() << "]\n";
        file << "  Читатель : " << userName  << " [" << r.getUserId() << "]\n";
        file << "  Выдана   : " << r.getIssueDate() << "\n";
        file << "  Вернуть  : " << r.getReturnDate() << "\n";
        file << "  " << std::string(40, '-') << "\n";
    }
    if (!hasActive) file << "  Нет активных выдач.\n";
    file << "\n";

    // 4. История возвратов
    file << "------------------------------------------------------------\n";
    file << "[ ИСТОРИЯ ВОЗВРАТОВ ]\n";
    file << "------------------------------------------------------------\n";
    bool hasHistory = false;
    for (const auto& r : m_issueRecords) {
        if (r.getReturnDate().empty()) continue;
        hasHistory = true;
        std::string bookTitle = r.getBookId();
        for (const auto& b : m_books)
            if (b.getBookId() == r.getBookId()) { bookTitle = b.getTitle(); break; }
        std::string userName = r.getUserId();
        for (const auto& u : m_users)
            if (u.getUserId() == r.getUserId()) { userName = u.getFullName(); break; }

        file << "  " << bookTitle << " | " << userName
             << " | Выд: " << r.getIssueDate()
             << " | Возвр: " << r.getReturnDate() << "\n";
    }
    if (!hasHistory) file << "  История возвратов пуста.\n";
    file << "\n";

    file << "============================================================\n";
    file << "  Конец отчёта\n";
    file << "============================================================\n";

    return true;
}