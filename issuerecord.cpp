#include "issuerecord.h"
#include <sstream>

IssueRecord::IssueRecord() : m_isOverdue(false) {}
IssueRecord::IssueRecord(std::string bookId, std::string userId, std::string issueDate,
                         std::string returnDate, bool isOverdue)
    : m_bookId(std::move(bookId)), m_userId(std::move(userId)),
    m_issueDate(std::move(issueDate)), m_returnDate(std::move(returnDate)),
    m_isOverdue(isOverdue) {}

std::string IssueRecord::getBookId() const { return m_bookId; }
std::string IssueRecord::getUserId() const { return m_userId; }
std::string IssueRecord::getIssueDate() const { return m_issueDate; }
std::string IssueRecord::getReturnDate() const { return m_returnDate; }
bool IssueRecord::getIsOverdue() const { return m_isOverdue; }

void IssueRecord::setReturnDate(std::string returnDate) { m_returnDate = std::move(returnDate); }
void IssueRecord::setIsOverdue(bool isOverdue) { m_isOverdue = isOverdue; }

std::string IssueRecord::toFileString() const {
    return m_bookId + "|" + m_userId + "|" + m_issueDate + "|" + m_returnDate + "|" + (m_isOverdue ? "1" : "0");
}

IssueRecord IssueRecord::fromFileString(const std::string& line) {
    std::istringstream ss(line);
    std::string bId, uId, iDate, rDate, overdueStr;
    std::getline(ss, bId, '|'); std::getline(ss, uId, '|');
    std::getline(ss, iDate, '|'); std::getline(ss, rDate, '|'); std::getline(ss, overdueStr);
    return IssueRecord(bId, uId, iDate, rDate, overdueStr == "1");
}