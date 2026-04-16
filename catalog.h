#pragma once
#include <vector>
#include <string>
#include "book.h"
#include "user.h"
#include "issuerecord.h"
#include <algorithm>// ← Обязательно для записей о выдачах

namespace LibraryConstants {
constexpr const char* kDataFileName = "library_data.txt";
constexpr int kMaxBooks = 1000;
constexpr int kMaxUsers = 500;
}

class Catalog {
public:
    Catalog();
    void loadData();
    void saveData() const;

    // Книги
    void addBook(const Book& book);
    bool removeBook(const std::string& bookId);   // ← Было void, стало bool
    bool removeUser(const std::string& userId);
    const std::vector<Book>& getBooks() const;

    // Читатели
    void addUser(const User& user);
    const std::vector<User>& getUsers() const;

    // Выдачи
    std::vector<IssueRecord> getUserHistory(const std::string& userId) const;

    std::string generateBookId(const std::string& genre) const;
    std::string generateUserId() const;

private:
    std::vector<Book> m_books;
    std::vector<User> m_users;         // ← Добавлено
    std::vector<IssueRecord> m_issueRecords; // ← Добавлено
};