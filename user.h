#pragma once
#include <string>

class User {
public:
    User();
    User(std::string userId, std::string fullName, std::string phoneNumber);

    std::string getUserId() const;
    std::string getFullName() const;
    std::string getPhoneNumber() const;

    void setFullName(std::string fullName);
    void setPhoneNumber(std::string phoneNumber);

    std::string toFileString() const;
    static User fromFileString(const std::string& line);

private:
    std::string m_userId;
    std::string m_fullName;
    std::string m_phoneNumber;
};