#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <string>
#include "Database.h"
#include "User.h"
#include "CreditScorer.h"

class CustomerWindow : public QMainWindow {
    Q_OBJECT

public:
    CustomerWindow(Database& database, const User& user, QWidget *parent = nullptr);

private slots:
    void handleWithdraw();
    void handleTransfer();
    void handleLoanApplication();
    void handleAcceptLoan();
    void handlePayLoan();
    void handleExportStatement();

private:
    Database& db;
    User currentUser;
    CreditScorer scorer;

    std::string customerAccountNumber;
    double currentAccountBalance;
    double offeredInterestRate;
    double lastRequestedAmount;
    int lastRequestedTerm;

    QLabel* welcomeLabel;
    QLabel* balanceLabel;
    QLabel* statusLabel;

    QLineEdit* withdrawAmountInput;
    QLineEdit* targetAccInput;
    QLineEdit* amountInput;

    QLineEdit* loanAgeInput;
    QLineEdit* loanIncomeInput;
    QLineEdit* loanAmountInput;
    QLineEdit* loanTermInput;
    QLabel* loanResultLabel;
    QPushButton* acceptLoanBtn;

    QTableWidget* loansTable;
    QLineEdit* payLoanAmountInput;
    QPushButton* payLoanBtn;

    QTableWidget* historyTable;

    void setupUI();
    void loadAccountData();
    void loadLoanData();
    void loadTransactionHistory();
    bool sendStatementEmail(const std::string& toEmail, const std::string& filePath);
};
