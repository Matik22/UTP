#pragma once
#include <QMainWindow>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QStackedWidget>
#include "catalog.h"

class ReaderMenuDialog;
class QPushButton;

class MainWindow : public QMainWindow {
 Q_OBJECT
public:
 explicit MainWindow(QWidget* parent = nullptr);
 ~MainWindow() override;

private slots:
 void onAddBook();
 void onSaveData();
 void onRemoveBookById();
 void onIssueBook();
 void onSearchChanged();
 void onShowChart();
 void onSwitchToReaders();
 void onSwitchToCatalog();
 void onRegisterReader();
 void onRemoveReaderBySurname();
 void onReaderSelectionChanged(const QModelIndex& current, const QModelIndex& previous);

private:
 void setupUI();
 void setupBooksPage();
 void setupReadersPage();
 void refreshTable();
 void refreshReadersTable();
 void showReaderHistory(const std::string& userId);

 Catalog m_catalog;
 QStackedWidget* m_stack;

 // Страница каталога
 QWidget* m_booksPage;
 QStandardItemModel* m_sourceModel;
 QSortFilterProxyModel* m_proxyModel;
 QTableView* m_tableView;
 QPushButton* m_btnAdd;
 QPushButton* m_btnSave;
 QPushButton* m_btnReaders;
 QPushButton* m_btnDeleteBook;
 QPushButton* m_btnIssueBook;
 QPushButton* m_btnChart;
 QLineEdit* m_searchEdit;
 QComboBox* m_searchColumn;

 // Страница читателей
 QWidget* m_readersPage;
 QStandardItemModel* m_userModel;
 QSortFilterProxyModel* m_userProxy;
 QStandardItemModel* m_historyModel;
 QTableView* m_userTable;
 QTableView* m_historyTable;
 QPushButton* m_btnBack;
 QPushButton* m_btnRegister;
 QPushButton* m_btnDeleteReader;
};
