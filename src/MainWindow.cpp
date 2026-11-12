#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>

MainWindow::MainWindow(Database& database, const User& user, QWidget *parent)
    : QMainWindow(parent), db(database), currentUser(user) {

    setWindowTitle(QString::fromStdString("Bank Management System - Dashboard (" + currentUser.getUsername() + ")"));
    resize(800, 560);

    tabWidget = new QTabWidget(this);
    setCentralWidget(tabWidget);

    QWidget* operationsTab = new QWidget();
    QWidget* historyTab = new QWidget();
    QWidget* loansTab = new QWidget();

    setupOperationsTab(operationsTab);
    setupHistoryTab(historyTab);
    setupLoansTab(loansTab);

    tabWidget->addTab(operationsTab, "Transactions & Operations");
    tabWidget->addTab(historyTab, "Account History");
    tabWidget->addTab(loansTab, "Loans Management");
}

void MainWindow::setupOperationsTab(QWidget* tab) {
    QVBoxLayout* layout = new QVBoxLayout(tab);

    fromAccInput = new QLineEdit();
    fromAccInput->setPlaceholderText("Source Account (e.g., AM01BANK0001)");
    layout->addWidget(new QLabel("From Account:"));
    layout->addWidget(fromAccInput);

    toAccInput = new QLineEdit();
    toAccInput->setPlaceholderText("Target Account (for transfer only)");
    layout->addWidget(new QLabel("To Account (optional):"));
    layout->addWidget(toAccInput);

    amountInput = new QLineEdit();
    amountInput->setPlaceholderText("Amount");
    layout->addWidget(new QLabel("Amount:"));
    layout->addWidget(amountInput);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    QPushButton* depBtn = new QPushButton("Deposit");
    QPushButton* withBtn = new QPushButton("Withdraw");
    QPushButton* transBtn = new QPushButton("Transfer");

    btnLayout->addWidget(depBtn);
    btnLayout->addWidget(withBtn);
    btnLayout->addWidget(transBtn);
    layout->addLayout(btnLayout);

    opStatusLabel = new QLabel("");
    opStatusLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(opStatusLabel);

    connect(depBtn, &QPushButton::clicked, this, &MainWindow::handleDeposit);
    connect(withBtn, &QPushButton::clicked, this, &MainWindow::handleWithdraw);
    connect(transBtn, &QPushButton::clicked, this, &MainWindow::handleTransfer);
}

void MainWindow::setupHistoryTab(QWidget* tab) {
    QVBoxLayout* layout = new QVBoxLayout(tab);

    QHBoxLayout* topLayout = new QHBoxLayout();
    historyAccInput = new QLineEdit();
    historyAccInput->setPlaceholderText("Enter Account Number (e.g., AM01BANK0001)");
    QPushButton* loadBtn = new QPushButton("View History");

    topLayout->addWidget(historyAccInput);
    topLayout->addWidget(loadBtn);
    layout->addLayout(topLayout);

    historyTable = new QTableWidget();
    historyTable->setColumnCount(3);
    historyTable->setHorizontalHeaderLabels({"Type", "Amount", "Date"});
    historyTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    historyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layout->addWidget(historyTable);

    connect(loadBtn, &QPushButton::clicked, this, &MainWindow::loadTransactions);
}

void MainWindow::setupLoansTab(QWidget* tab) {
    QVBoxLayout* layout = new QVBoxLayout(tab);

    QHBoxLayout* searchLayout = new QHBoxLayout();
    loanCustomerIdInput = new QLineEdit();
    loanCustomerIdInput->setPlaceholderText("Enter Customer ID (e.g. 1)");
    searchLoansBtn = new QPushButton("Search Loans");

    searchLayout->addWidget(loanCustomerIdInput);
    searchLayout->addWidget(searchLoansBtn);
    layout->addLayout(searchLayout);

    adminLoansTable = new QTableWidget();
    adminLoansTable->setColumnCount(6);
    adminLoansTable->setHorizontalHeaderLabels({"Loan ID", "Amount", "Remaining Debt", "Monthly Pay", "Term (M)", "Status"});
    adminLoansTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    adminLoansTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layout->addWidget(adminLoansTable);

    connect(searchLoansBtn, &QPushButton::clicked, this, &MainWindow::handleSearchCustomerLoans);
}

void MainWindow::handleDeposit() {
    std::string acc = fromAccInput->text().toStdString();
    double amount = amountInput->text().toDouble();

    if (acc.empty() || amount <= 0) {
        opStatusLabel->setStyleSheet("color: red;");
        opStatusLabel->setText("Invalid account or amount!");
        return;
    }

    try {
        db.deposit(acc, amount);
        opStatusLabel->setStyleSheet("color: green;");
        opStatusLabel->setText("Deposit successful!");
    } catch (const std::exception& e) {
        opStatusLabel->setStyleSheet("color: red;");
        opStatusLabel->setText(e.what());
    }
}

void MainWindow::handleWithdraw() {
    std::string acc = fromAccInput->text().toStdString();
    double amount = amountInput->text().toDouble();

    if (acc.empty() || amount <= 0) {
        opStatusLabel->setStyleSheet("color: red;");
        opStatusLabel->setText("Invalid account or amount!");
        return;
    }

    bool success = db.withdraw(acc, amount);

    if (success) {
        opStatusLabel->setStyleSheet("color: green;");
        opStatusLabel->setText("Withdraw successful!");
        amountInput->clear();
    } else {
        opStatusLabel->setStyleSheet("color: red;");
        opStatusLabel->setText("Withdraw failed! Account not found or insufficient funds.");
    }
}

void MainWindow::handleTransfer() {
    std::string fromAcc = fromAccInput->text().toStdString();
    std::string toAcc = toAccInput->text().toStdString();
    double amount = amountInput->text().toDouble();

    if (fromAcc.empty() || toAcc.empty() || amount <= 0) {
        opStatusLabel->setStyleSheet("color: red;");
        opStatusLabel->setText("Fill all fields correctly!");
        return;
    }

    if (fromAcc == toAcc) {
        opStatusLabel->setStyleSheet("color: red;");
        opStatusLabel->setText("Cannot transfer to the same account!");
        return;
    }

    bool success = db.transfer(fromAcc, toAcc, amount);

    if (success) {
        opStatusLabel->setStyleSheet("color: green;");
        opStatusLabel->setText("Transfer successful!");
        amountInput->clear();
    } else {
        opStatusLabel->setStyleSheet("color: red;");
        opStatusLabel->setText("Transfer failed! Check account numbers or balance.");
    }
}

void MainWindow::loadTransactions() {
    std::string acc = historyAccInput->text().toStdString();
    if (acc.empty()) return;

    try {
        std::vector<Transaction> transactions = db.getTransactionHistory(acc);
        historyTable->setRowCount(0);

        for (const auto& tx : transactions) {
            int row = historyTable->rowCount();
            historyTable->insertRow(row);
            historyTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(tx.getType())));
            historyTable->setItem(row, 1, new QTableWidgetItem(QString::number(tx.getAmount())));
            historyTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(tx.getDate())));
        }
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error", e.what());
    }
}

void MainWindow::handleSearchCustomerLoans() {
    bool ok;
    int customerId = loanCustomerIdInput->text().toInt(&ok);

    if (!ok || customerId <= 0) {
        QMessageBox::warning(this, "Input Error", "Please enter a valid Customer ID.");
        return;
    }

    auto loans = db.getCustomerLoans(customerId);
    adminLoansTable->setRowCount(0);

    if (loans.empty()) {
        QMessageBox::information(this, "No Loans", "No loan records found for Customer ID: " + QString::number(customerId));
        return;
    }

    for (const auto& loan : loans) {
        int row = adminLoansTable->rowCount();
        adminLoansTable->insertRow(row);
        adminLoansTable->setItem(row, 0, new QTableWidgetItem(QString::number(loan.id)));
        adminLoansTable->setItem(row, 1, new QTableWidgetItem(QString::number(loan.amount, 'f', 2) + " AMD"));
        adminLoansTable->setItem(row, 2, new QTableWidgetItem(QString::number(loan.remainingAmount, 'f', 2) + " AMD"));
        adminLoansTable->setItem(row, 3, new QTableWidgetItem(QString::number(loan.monthlyPayment, 'f', 2) + " AMD"));
        adminLoansTable->setItem(row, 4, new QTableWidgetItem(QString::number(loan.termMonths)));

        QTableWidgetItem* statusItem = new QTableWidgetItem(QString::fromStdString(loan.status));
        if (loan.status == "ACTIVE") {
            statusItem->setForeground(QBrush(QColor("#d32f2f")));
        } else {
            statusItem->setForeground(QBrush(QColor("#2e7d32")));
        }
        adminLoansTable->setItem(row, 5, statusItem);
    }
}
