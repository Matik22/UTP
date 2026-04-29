#pragma once
#include <vector>
#include <string>
#include "book.h"
#include "user.h"
#include "issuerecord.h"
#include <QString>


namespace LibraryConstants {
constexpr const char* kDataFileName = "library_data.txt";
constexpr const char* uDataFileName = "user_data.txt";
constexpr int kMaxBooks = 1000;
constexpr int kMaxUsers = 500;
constexpr int kMaxBorrowDays = 60;
}

class Catalog {
public:
    Catalog();
    void loadData();
    void saveData() const;
    void saveUser() const;

    void addBook(const Book& book);
    bool removeBook(const std::string& bookId);
    bool removeUser(const std::string& userId);
    const std::vector<Book>& getBooks() const;

    void addUser(const User& user);
    const std::vector<User>& getUsers() const;

    std::vector<IssueRecord> getUserHistory(const std::string& userId) const;

    std::string generateBookId(const std::string& genre) const;
    std::string generateUserId() const;

    // Выдача и возврат
    bool issueBook(const std::string& bookId, const std::string& userId,
                   const std::string& issueDate, const std::string& returnDate);
    bool returnBook(const std::string& bookId, const std::string& returnDate);
    bool isBookIssued(const std::string& bookId) const;
    std::vector<Book> getAvailableBooks() const;
    const std::vector<IssueRecord>& getIssueRecords() const;

private:
    std::vector<Book> m_books;
    std::vector<User> m_users;
    std::vector<IssueRecord> m_issueRecords;
    QString getDataFilePath() const;

};