#pragma once
#include <QMainWindow>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QTableView>      // <-- Добавлено
#include <QPushButton>     // <-- Добавлено
#include "catalog.h"
// В начало файла
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

private:
    void setupUI();
    void refreshTable();

    Catalog m_catalog;
    QStandardItemModel* m_sourceModel;
    QSortFilterProxyModel* m_proxyModel;

    QTableView* m_tableView;
    QPushButton* m_btnAdd;
    QPushButton* m_btnSave;
    // В private секцию
    void openReaderMenu(); // ← Новый слот
    QPushButton* m_btnReaders; // ← Новая кнопка
    QPushButton* m_btnDeleteBook;

};