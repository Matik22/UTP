#pragma once
#include <QMainWindow>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
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

private:
 void setupUI();
 void refreshTable();

 Catalog m_catalog;
 QStandardItemModel* m_sourceModel;
 QSortFilterProxyModel* m_proxyModel;

 QTableView* m_tableView;
 QPushButton* m_btnAdd;
 QPushButton* m_btnSave;

 void openReaderMenu();
 QPushButton* m_btnReaders;
 QPushButton* m_btnDeleteBook;

 QPushButton* m_btnIssueBook;

 QLineEdit* m_searchEdit;
 QComboBox* m_searchColumn;
};
