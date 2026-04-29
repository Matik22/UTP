#pragma once
#include <string>

class Book {
public:
    Book();

    Book(std::string bookId, std::string title, std::string author,
         std::string publisher, int year, std::string genre);

    std::string getBookId() const;
    std::string getTitle() const;
    std::string getAuthor() const;
    std::string getPublisher() const;
    int getYear() const;
    std::string getGenre() const;

    void setTitle(std::string title);
    void setAuthor(std::string author);
    void setPublisher(std::string publisher);
    void setYear(int year);
    void setGenre(std::string genre);

    std::string toFileString() const;
    static Book fromFileString(const std::string& line);

private:
    std::string m_bookId;
    std::string m_title;
    std::string m_author;
    std::string m_publisher;
    int m_year;
    std::string m_genre;
};