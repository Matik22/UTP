#pragma once
#include <string>

class IssueRecord {
public:
    IssueRecord();
    IssueRecord(std::string bookId, std::string userId, std::string issueDate,
                std::string returnDate, bool isOverdue);

    std::string getBookId() const;
    std::string getUserId() const;
    std::string getIssueDate() const;
    std::string getReturnDate() const;
    bool getIsOverdue() const;

    void setReturnDate(std::string returnDate);
    void setIsOverdue(bool isOverdue);

    std::string toFileString() const;
    static IssueRecord fromFileString(const std::string& line);

private:
    std::string m_bookId;
    std::string m_userId;
    std::string m_issueDate;
    std::string m_returnDate;
    bool m_isOverdue;
};