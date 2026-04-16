#include "book.h"
#include <sstream>

Book::Book() : m_year(0) {}
Book::Book(std::string bookId, std::string title, std::string author,
           std::string publisher, int year, std::string genre)
    : m_bookId(std::move(bookId)), m_title(std::move(title)),
    m_author(std::move(author)), m_publisher(std::move(publisher)),
    m_year(year), m_genre(std::move(genre)) {}

std::string Book::getBookId() const { return m_bookId; }
std::string Book::getTitle() const { return m_title; }
std::string Book::getAuthor() const { return m_author; }
std::string Book::getPublisher() const { return m_publisher; }
int Book::getYear() const { return m_year; }
std::string Book::getGenre() const { return m_genre; }

void Book::setTitle(std::string title) { m_title = std::move(title); }
void Book::setAuthor(std::string author) { m_author = std::move(author); }
void Book::setPublisher(std::string publisher) { m_publisher = std::move(publisher); }
void Book::setYear(int year) { m_year = year; }
void Book::setGenre(std::string genre) { m_genre = std::move(genre); }

std::string Book::toFileString() const {
    return m_bookId + "|" + m_title + "|" + m_author + "|" + m_publisher + "|" +
           std::to_string(m_year) + "|" + m_genre;
}

Book Book::fromFileString(const std::string& line) {
    std::istringstream ss(line);
    std::string id, title, author, pub, yearStr, genre;
    std::getline(ss, id, '|'); std::getline(ss, title, '|');
    std::getline(ss, author, '|'); std::getline(ss, pub, '|');
    std::getline(ss, yearStr, '|'); std::getline(ss, genre);
    return Book(id, title, author, pub, std::stoi(yearStr), genre);
}