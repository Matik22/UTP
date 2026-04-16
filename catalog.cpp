#include "catalog.h"
#include <fstream>
#include <iostream>
#include <QCoreApplication>
#include <QDebug>
#include <sstream>
#include <iomanip>

Catalog::Catalog() = default;

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
    if (isIssued) return false; // Нельзя удалить книгу в обороте

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
    // Ищем файл рядом с .exe
    QString dirPath = QCoreApplication::applicationDirPath();
    QString fullPath = dirPath + "/" + LibraryConstants::kDataFileName;

    std::ifstream file(fullPath.toStdString());
    if (!file.is_open()) {
        qDebug() << "[WARN] Файл не найден по пути:" << fullPath;
        qDebug() << "[INFO] Положите library_data.txt в папку:" << dirPath;
        return;
    }

    std::string line;
    int loaded = 0;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#' || line.find("===") == 0) continue;
        try {
            Book b = Book::fromFileString(line);
            if (m_books.size() < LibraryConstants::kMaxBooks) {
                m_books.push_back(b);
                loaded++;
            }
        } catch (...) {
            qDebug() << "Ошибка парсинга строки:" << QString::fromStdString(line);
        }
    }
    qDebug() << "Загружено книг:" << loaded << "из" << fullPath;
}

void Catalog::saveData() const {
    std::ofstream file(LibraryConstants::kDataFileName, std::ios::trunc);
    for (const auto& b : m_books) file << b.toFileString() << "\n";
}

const std::vector<Book>& Catalog::getBooks() const { return m_books; }

// ... ваш существующий код loadData и saveData ...

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