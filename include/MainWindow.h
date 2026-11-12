#pragma once

#include <QMainWindow>
#include <QTabWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>
#include "Database.h"
#include "User.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(Database& database, const User& user, QWidget *parent = nullptr);

private slots:
    void handleDeposit();
    void handleWithdraw();
    void handleTransfer();
    void loadTransactions();
    void handleSearchCustomerLoans();

private:
    Database& db;
    User currentUser;

    QTabWidget* tabWidget;

    QLineEdit* fromAccInput;
    QLineEdit* toAccInput;
    QLineEdit* amountInput;
    QLabel* opStatusLabel;

    QLineEdit* historyAccInput;
    QTableWidget* historyTable;

    QLineEdit* loanCustomerIdInput;
    QPushButton* searchLoansBtn;
    QTableWidget* adminLoansTable;

    void setupOperationsTab(QWidget* tab);
    void setupHistoryTab(QWidget* tab);
    void setupLoansTab(QWidget* tab);
};
