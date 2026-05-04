#pragma once
#include <string>

class Librarian {
public:
    Librarian() = default;
    Librarian(std::string login, std::string password)
        : m_login(std::move(login)), m_password(std::move(password)) {}

    const std::string& login() const { return m_login; }
    const std::string& password() const { return m_password; }

    std::string toFileString() const {
        return m_login + "|" + m_password;
    }

    static Librarian fromFileString(const std::string& line) {
        auto pos = line.find('|');
        return Librarian(line.substr(0, pos), line.substr(pos + 1));
    }

private:
    std::string m_login;
    std::string m_password;
};
