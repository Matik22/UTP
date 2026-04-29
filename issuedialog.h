#pragma once
#include <QDialog>

class Catalog;
class QComboBox; class QDateEdit; class QPushButton; class QFormLayout;

class IssueDialog : public QDialog {
    Q_OBJECT
public:
    explicit IssueDialog(Catalog& catalog, QWidget* parent = nullptr);
    ~IssueDialog() override;

private slots:
    void onIssueConfirmed();
    void onIssueDateChanged(const QDate& date);


private:
    void setupUI();
    void populateBooks();
    void populateUsers();
    void updateReturnDate();

    Catalog& m_catalog;
    QComboBox* m_bookCombo;
    QComboBox* m_userCombo;
    QDateEdit* m_issueDateEdit;
    QDateEdit* m_returnDateEdit;
};