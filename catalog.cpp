#include "catalog.h"
#include <fstream>
#include <iostream>
#include <QCoreApplication>
#include <QDebug>
#include <sstream>
#include <iomanip>
#include <algorithm>

Catalog::Catalog() = default;

QString Catalog::getDataFilePath() const {
    return QCoreApplication::applicationDirPath() + "/" + LibraryConstants::kDataFileName;
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
    char prefix = std::toupper(genre.empty() ? 'X' : genre[0]);
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