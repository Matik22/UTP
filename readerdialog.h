#pragma once
#include <QDialog>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>

class Catalog;
class QVBoxLayout; class QHBoxLayout; class QTableView; class QPushButton; class QLineEdit;

class ReaderMenuDialog : public QDialog {
    Q_OBJECT
public:
    explicit ReaderMenuDialog(Catalog& catalog, QWidget* parent = nullptr);
    ~ReaderMenuDialog() override;

private slots:
    void onRegisterReader();
    void onRemoveReaderBySurname();
    void onReaderSelectionChanged(const QModelIndex& current, const QModelIndex& previous);

private:
    void setupUI();
    void outputReader();
    void issueHistory(const std::string& userId);

    Catalog& m_catalog;
    QStandardItemModel* m_userModel;
    QSortFilterProxyModel* m_userProxy;
    QStandardItemModel* m_historyModel;

    QTableView* m_userTable;
    QTableView* m_historyTable;
    QPushButton* m_btnRegister;
    QPushButton* m_btnDelete;
};