#include "user.h"
#include <sstream>

User::User() = default;
User::User(std::string userId, std::string fullName, std::string phoneNumber)
    : m_userId(std::move(userId)), m_fullName(std::move(fullName)), m_phoneNumber(std::move(phoneNumber)) {}

std::string User::getUserId() const { return m_userId; }
std::string User::getFullName() const { return m_fullName; }
std::string User::getPhoneNumber() const { return m_phoneNumber; }

void User::setFullName(std::string fullName) { m_fullName = std::move(fullName); }
void User::setPhoneNumber(std::string phoneNumber) { m_phoneNumber = std::move(phoneNumber); }

std::string User::toFileString() const {
    return m_userId + "|" + m_fullName + "|" + m_phoneNumber;
}

User User::fromFileString(const std::string& line) {
    std::istringstream ss(line);
    std::string id, name, phone;
    std::getline(ss, id, '|'); std::getline(ss, name, '|'); std::getline(ss, phone);
    return User(id, name, phone);
}