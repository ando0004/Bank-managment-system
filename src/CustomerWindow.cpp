#include "CustomerWindow.h"
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QTextDocument>
#include <QtPrintSupport/QPrinter>
#include <QMessageBox>
#include <curl/curl.h>
#include <cstdlib>
#include <QInputDialog>
#include <QDebug>
CustomerWindow::CustomerWindow(Database& database, const User& user, QWidget *parent)
    : QMainWindow(parent), db(database), currentUser(user), currentAccountBalance(0.0),
      offeredInterestRate(0.0), lastRequestedAmount(0.0), lastRequestedTerm(0) {
    setupUI();
    loadAccountData();
    loadLoanData();
}

void CustomerWindow::setupUI() {
    setWindowTitle(QString::fromStdString("Bank Client Cabinet - " + currentUser.getUsername()));
    resize(750, 900);

    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    welcomeLabel = new QLabel(QString::fromStdString("Welcome, " + currentUser.getUsername() + "!"));
    welcomeLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    mainLayout->addWidget(welcomeLabel);

    balanceLabel = new QLabel("Account: Loading... | Balance: Loading...");
    balanceLabel->setStyleSheet("font-size: 14px; color: #2e7d32; font-weight: bold;");
    mainLayout->addWidget(balanceLabel);

    mainLayout->addWidget(new QLabel("<b>Quick Withdraw:</b>"));
    QHBoxLayout* withdrawLayout = new QHBoxLayout();
    withdrawAmountInput = new QLineEdit();
    withdrawAmountInput->setPlaceholderText("Amount to withdraw");
    QPushButton* withdrawBtn = new QPushButton("Withdraw Cash");
    withdrawLayout->addWidget(withdrawAmountInput);
    withdrawLayout->addWidget(withdrawBtn);
    mainLayout->addLayout(withdrawLayout);

    mainLayout->addWidget(new QLabel("<b>Quick Transfer:</b>"));
    QHBoxLayout* transferLayout = new QHBoxLayout();
    targetAccInput = new QLineEdit();
    targetAccInput->setPlaceholderText("Recipient Account (e.g., AM01BANK0002)");
    amountInput = new QLineEdit();
    amountInput->setPlaceholderText("Amount to transfer");
    QPushButton* sendBtn = new QPushButton("Send Money");

    transferLayout->addWidget(targetAccInput);
    transferLayout->addWidget(amountInput);
    transferLayout->addWidget(sendBtn);
    mainLayout->addLayout(transferLayout);

    statusLabel = new QLabel("");
    statusLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(statusLabel);

    QGroupBox* loanBox = new QGroupBox("Loan Eligibility & Application (AI Scorer)");
    QVBoxLayout* loanBoxLayout = new QVBoxLayout(loanBox);

    QHBoxLayout* loanInputsLayout1 = new QHBoxLayout();
    loanAgeInput = new QLineEdit();
    loanAgeInput->setPlaceholderText("Your Age (e.g., 28)");
    loanIncomeInput = new QLineEdit();
    loanIncomeInput->setPlaceholderText("Monthly Income (AMD)");
    loanInputsLayout1->addWidget(loanAgeInput);
    loanInputsLayout1->addWidget(loanIncomeInput);
    loanBoxLayout->addLayout(loanInputsLayout1);

    QHBoxLayout* loanInputsLayout2 = new QHBoxLayout();
    loanAmountInput = new QLineEdit();
    loanAmountInput->setPlaceholderText("Requested Loan Amount (AMD)");
    loanTermInput = new QLineEdit();
    loanTermInput->setPlaceholderText("Term in Months (e.g., 24)");
    loanInputsLayout2->addWidget(loanAmountInput);
    loanInputsLayout2->addWidget(loanTermInput);
    loanBoxLayout->addLayout(loanInputsLayout2);

    QPushButton* applyLoanBtn = new QPushButton("Check Loan Eligibility");
    loanBoxLayout->addWidget(applyLoanBtn);

    loanResultLabel = new QLabel("Enter parameters above to check credit score.");
    loanResultLabel->setAlignment(Qt::AlignCenter);
    loanResultLabel->setStyleSheet("font-weight: bold; padding: 5px;");
    loanBoxLayout->addWidget(loanResultLabel);

    acceptLoanBtn = new QPushButton("Accept Loan");
    acceptLoanBtn->setEnabled(false);
    acceptLoanBtn->setStyleSheet("background-color: #2e7d32; color: white; font-weight: bold; padding: 6px;");
    loanBoxLayout->addWidget(acceptLoanBtn);

    mainLayout->addWidget(loanBox);

    mainLayout->addWidget(new QLabel("<b>Your Loans Overview:</b>"));
    loansTable = new QTableWidget();
    loansTable->setColumnCount(6);
    loansTable->setHorizontalHeaderLabels({"Loan ID", "Initial Amount", "Remaining Debt", "Monthly Pay", "Term (M)", "Status"});
    loansTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    loansTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    loansTable->setSelectionMode(QAbstractItemView::SingleSelection);
    loansTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mainLayout->addWidget(loansTable);

    QHBoxLayout* payLoanLayout = new QHBoxLayout();
    payLoanAmountInput = new QLineEdit();
    payLoanAmountInput->setPlaceholderText("Enter payment amount to repay selected loan");
    payLoanBtn = new QPushButton("Pay Selected Loan");
    payLoanBtn->setStyleSheet("background-color: #1976d2; color: white; font-weight: bold; padding: 5px;");

    payLoanLayout->addWidget(payLoanAmountInput);
    payLoanLayout->addWidget(payLoanBtn);
    mainLayout->addLayout(payLoanLayout);

    mainLayout->addWidget(new QLabel("<b>Your Transaction History:</b>"));
    historyTable = new QTableWidget();
    historyTable->setColumnCount(3);
    historyTable->setHorizontalHeaderLabels({"Type", "Amount", "Date"});
    historyTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    historyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mainLayout->addWidget(historyTable);

    QPushButton* exportStatementBtn = new QPushButton("Export Statement (PDF / CSV)");
    exportStatementBtn->setStyleSheet("background-color: #2e7d32; color: white; font-weight: bold; padding: 6px 12px; border-radius: 4px;");
    mainLayout->addWidget(exportStatementBtn);

    connect(exportStatementBtn, &QPushButton::clicked, this, &CustomerWindow::handleExportStatement);

    connect(withdrawBtn, &QPushButton::clicked, this, &CustomerWindow::handleWithdraw);
    connect(sendBtn, &QPushButton::clicked, this, &CustomerWindow::handleTransfer);
    connect(applyLoanBtn, &QPushButton::clicked, this, &CustomerWindow::handleLoanApplication);
    connect(acceptLoanBtn, &QPushButton::clicked, this, &CustomerWindow::handleAcceptLoan);
    connect(payLoanBtn, &QPushButton::clicked, this, &CustomerWindow::handlePayLoan);

    connect(loansTable, &QTableWidget::itemClicked, this, [this](QTableWidgetItem *item) {
        if (!item) return;
        int row = item->row();
        QTableWidgetItem *monthlyPayItem = loansTable->item(row, 3);
        if (monthlyPayItem) {
            payLoanAmountInput->setText(monthlyPayItem->text().split(" ").first());
        }
    });
}
void CustomerWindow::loadAccountData() {
    try {
        customerAccountNumber = db.getAccountNumberByCustomerId((currentUser.getCustomerId() > 0 ? currentUser.getCustomerId() : currentUser.getId()));

        if (customerAccountNumber.empty()) {
            statusLabel->setStyleSheet("color: red;");
            statusLabel->setText("No active bank account associated with this user.");
            balanceLabel->setText("Account: None | Balance: 0.00 AMD");
            currentAccountBalance = 0.0;
            return;
        }

        Account acc = db.getAccountByNumber(customerAccountNumber);
        currentAccountBalance = acc.getBalance();

        balanceLabel->setText(QString("Account: %1 | Balance: %2 AMD")
                                  .arg(QString::fromStdString(customerAccountNumber))
                                  .arg(currentAccountBalance, 0, 'f', 2));

        auto history = db.getTransactionHistory(customerAccountNumber);
        historyTable->setRowCount(0);

        for (const auto& tx : history) {
            int row = historyTable->rowCount();
            historyTable->insertRow(row);
            historyTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(tx.getType())));
            historyTable->setItem(row, 1, new QTableWidgetItem(QString::number(tx.getAmount(), 'f', 2)));
            historyTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(tx.getDate())));
        }

    } catch (const std::exception& e) {
        statusLabel->setStyleSheet("color: red;");
        statusLabel->setText(QString("Error: %1").arg(e.what()));
    }
}

void CustomerWindow::handleWithdraw() {
    if (customerAccountNumber.empty()) {
        statusLabel->setStyleSheet("color: red;");
        statusLabel->setText("No linked account found!");
        return;
    }

    bool ok;
    double amount = withdrawAmountInput->text().toDouble(&ok);

    if (!ok || amount <= 0) {
        statusLabel->setStyleSheet("color: red;");
        statusLabel->setText("Enter a valid positive amount to withdraw!");
        return;
    }

    bool success = db.withdraw(customerAccountNumber, amount);

    if (success) {
        statusLabel->setStyleSheet("color: green;");
        statusLabel->setText("Withdraw completed successfully!");
        withdrawAmountInput->clear();
        loadAccountData();
    } else {
        statusLabel->setStyleSheet("color: red;");
        statusLabel->setText("Withdraw failed! Insufficient funds or database error.");
    }
}

void CustomerWindow::handleTransfer() {
    if (customerAccountNumber.empty()) {
        statusLabel->setStyleSheet("color: red;");
        statusLabel->setText("No linked account found!");
        return;
    }

    std::string toAcc = targetAccInput->text().trimmed().toStdString();
    bool ok;
    double amount = amountInput->text().toDouble(&ok);

    if (toAcc.empty() || !ok || amount <= 0) {
        statusLabel->setStyleSheet("color: red;");
        statusLabel->setText("Invalid recipient or amount!");
        return;
    }

    if (customerAccountNumber == toAcc) {
        statusLabel->setStyleSheet("color: red;");
        statusLabel->setText("Cannot transfer to your own account!");
        return;
    }

    bool success = db.transfer(customerAccountNumber, toAcc, amount);

    if (success) {
        statusLabel->setStyleSheet("color: green;");
        statusLabel->setText("Transfer completed successfully!");
        amountInput->clear();
        targetAccInput->clear();
        loadAccountData();
    } else {
        statusLabel->setStyleSheet("color: red;");
        statusLabel->setText("Transfer failed! Check recipient account or balance.");
    }
}

void CustomerWindow::handleLoanApplication() {
    bool okAge, okIncome, okAmount, okTerm;
    int age = loanAgeInput->text().toInt(&okAge);
    double income = loanIncomeInput->text().toDouble(&okIncome);
    double requestedAmount = loanAmountInput->text().toDouble(&okAmount);
    int term = loanTermInput->text().toInt(&okTerm);

    if (!okAge || !okIncome || !okAmount || !okTerm || age < 18 || income <= 0 || requestedAmount <= 0 || term <= 0) {
        loanResultLabel->setStyleSheet("color: red; font-weight: bold;");
        loanResultLabel->setText("Please enter valid parameters (Age >= 18, positive amounts/terms).");
        acceptLoanBtn->setEnabled(false);
        return;
    }

    CreditApplicant applicant;
    applicant.age = age;
    applicant.monthlyIncome = income;
    applicant.currentBalance = currentAccountBalance;
    applicant.loanAmount = requestedAmount;
    applicant.loanTermMonths = term;

    CreditAssessment result = scorer.evaluate(applicant);

    lastRequestedAmount = requestedAmount;
    lastRequestedTerm = term;

    if (result.isApproved) {
        if (result.score >= 750) {
            offeredInterestRate = 11.5;
        } else if (result.score >= 670) {
            offeredInterestRate = 14.0;
        } else {
            offeredInterestRate = 18.5;
        }

        loanResultLabel->setStyleSheet("color: #2e7d32; font-weight: bold;");
        loanResultLabel->setText(QString("APPROVED! Score: %1/850 | Rate: %2% | Risk: %3")
                                     .arg(result.score)
                                     .arg(offeredInterestRate, 0, 'f', 1)
                                     .arg(QString::fromStdString(result.riskLevel)));
        acceptLoanBtn->setEnabled(true);
    } else {
        loanResultLabel->setStyleSheet("color: #c62828; font-weight: bold;");
        loanResultLabel->setText(QString("REJECTED! Score: %1/850 | Risk: %2")
                                     .arg(result.score)
                                     .arg(QString::fromStdString(result.riskLevel)));
        acceptLoanBtn->setEnabled(false);
    }
}

void CustomerWindow::handleAcceptLoan() {
    if (customerAccountNumber.empty()) {
        QMessageBox::warning(this, "Error", "No linked bank account found!");
        return;
    }

    bool success = db.applyAndDisburseLoan(
        (currentUser.getCustomerId() > 0 ? currentUser.getCustomerId() : currentUser.getId()),
        customerAccountNumber,
        lastRequestedAmount,
        lastRequestedTerm,
        offeredInterestRate
    );

    if (success) {
        QMessageBox::information(this, "Success", "Loan disbursed successfully! Funds added to your balance.");
        acceptLoanBtn->setEnabled(false);
        loanResultLabel->setText("Loan accepted and disbursed.");
        loadAccountData();
        loadLoanData();
    } else {
        QMessageBox::critical(this, "Error", "Failed to disburse loan. Check database connection.");
    }
}
void CustomerWindow::handlePayLoan() {
    int currentRow = loansTable->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "Selection Error", "Please select a loan from the table first.");
        return;
    }

    QTableWidgetItem* idItem = loansTable->item(currentRow, 0);
    if (!idItem) {
        QMessageBox::warning(this, "Selection Error", "Invalid loan selection.");
        return;
    }

    int loanId = idItem->text().toInt();

    bool ok = false;
    double amount = payLoanAmountInput->text().toDouble(&ok);
    if (!ok || amount <= 0.0) {
        QMessageBox::warning(this, "Input Error", "Please enter a valid positive payment amount.");
        return;
    }

    bool success = db.payLoan(loanId, (currentUser.getCustomerId() > 0 ? currentUser.getCustomerId() : currentUser.getId()), amount);
    if (success) {
        QMessageBox::information(this, "Success", "Payment processed successfully!");
        payLoanAmountInput->clear();
        loadLoanData();
        loadAccountData();
        loadTransactionHistory();
    } else {
        QMessageBox::critical(this, "Failure", "Payment failed. Insufficient funds or loan already paid.");
    }
}
void CustomerWindow::loadLoanData() {
    auto loans = db.getCustomerLoans((currentUser.getCustomerId() > 0 ? currentUser.getCustomerId() : currentUser.getId()));
    loansTable->setRowCount(0);

    for (const auto& loan : loans) {
        int row = loansTable->rowCount();
        loansTable->insertRow(row);

        loansTable->setItem(row, 0, new QTableWidgetItem(QString::number(loan.id)));
        loansTable->setItem(row, 1, new QTableWidgetItem(QString::number(loan.amount, 'f', 2) + " AMD"));
        loansTable->setItem(row, 2, new QTableWidgetItem(QString::number(loan.remainingAmount, 'f', 2) + " AMD"));
        loansTable->setItem(row, 3, new QTableWidgetItem(QString::number(loan.monthlyPayment, 'f', 2) + " AMD"));
        loansTable->setItem(row, 4, new QTableWidgetItem(QString::number(loan.termMonths)));

        QTableWidgetItem* statusItem = new QTableWidgetItem(QString::fromStdString(loan.status));
        if (loan.status == "ACTIVE") {
            statusItem->setForeground(QBrush(QColor("#d32f2f")));
        } else {
            statusItem->setForeground(QBrush(QColor("#2e7d32")));
        }
        loansTable->setItem(row, 5, statusItem);
    }
}

void CustomerWindow::loadTransactionHistory() {
    if (!historyTable) return;

    historyTable->setRowCount(0);

    std::string accNumber = db.getAccountNumberByCustomerId((currentUser.getCustomerId() > 0 ? currentUser.getCustomerId() : currentUser.getId()));
    if (accNumber.empty()) return;

    auto transactions = db.getTransactionHistory(accNumber);

    for (const auto& tx : transactions) {
        int row = historyTable->rowCount();
        historyTable->insertRow(row);

        historyTable->setItem(row, 0, new QTableWidgetItem(QString::number(tx.getId())));
        historyTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(tx.getType())));
        historyTable->setItem(row, 2, new QTableWidgetItem(QString::number(tx.getAmount(), 'f', 2)));
        historyTable->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(tx.getDate())));
    }
}

void CustomerWindow::handleExportStatement() {
    std::string accNumber = db.getAccountNumberByCustomerId((currentUser.getCustomerId() > 0 ? currentUser.getCustomerId() : currentUser.getId()));
    if (accNumber.empty()) {
        QMessageBox::warning(this, "Export Error", "No active account found for current customer.");
        return;
    }

    auto transactions = db.getTransactionHistory(accNumber);
    if (transactions.empty()) {
        QMessageBox::information(this, "Export Statement", "No transactions found to export.");
        return;
    }

    QString timeStamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString defaultFileName = QString("Statement_%1_%2").arg(QString::fromStdString(accNumber)).arg(timeStamp);

    QString selectedFilter;
    QString filePath = QFileDialog::getSaveFileName(
        this,
        "Save Bank Statement",
        defaultFileName,
        "PDF Files (*.pdf);;CSV Files (*.csv)",
        &selectedFilter
    );
    if (filePath.isEmpty()) {
        return;
    }

    if (selectedFilter.contains("*.pdf") || filePath.endsWith(".pdf", Qt::CaseInsensitive)) {
        if (!filePath.endsWith(".pdf", Qt::CaseInsensitive)) {
            filePath += ".pdf";
        }

        QString html = "<html><head><style>"
                       "body { font-family: Arial, sans-serif; margin: 20px; }"
                       "h2 { color: #1976d2; }"
                       "table { width: 100%; border-collapse: collapse; margin-top: 15px; }"
                       "th, td { border: 1px solid #ccc; padding: 8px; text-align: left; }"
                       "th { background-color: #f2f2f2; }"
                       "</style></head><body>";

        html += "<h2>Bank Account Statement</h2>";
        html += "<p><b>Account Number:</b> " + QString::fromStdString(accNumber) + "<br>";
        html += "<b>Export Date:</b> " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "</p>";

        html += "<table>";
        html += "<tr><th>ID</th><th>Type</th><th>Amount (AMD)</th><th>Date</th></tr>";

        for (const auto& tx : transactions) {
            html += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td></tr>")
                        .arg(tx.getId())
                        .arg(QString::fromStdString(tx.getType()))
                        .arg(QString::number(tx.getAmount(), 'f', 2))
                        .arg(QString::fromStdString(tx.getDate()));
        }
        html += "</table></body></html>";

        QTextDocument doc;
        doc.setHtml(html);

        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(filePath);

        doc.print(&printer);
    } else {
        if (!filePath.endsWith(".csv", Qt::CaseInsensitive)) {
            filePath += ".csv";
        }

        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(this, "Export Error", "Unable to open file: " + file.errorString());
            return;
        }

        QTextStream out(&file);
        out << "Bank Account Statement\n";
        out << "Account Number: " << QString::fromStdString(accNumber) << "\n";
        out << "Export Date: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n\n";
        out << "Transaction ID,Type,Amount (AMD),Date\n";

        for (const auto& tx : transactions) {
            out << tx.getId() << ","
                << QString::fromStdString(tx.getType()) << ","
                << QString::number(tx.getAmount(), 'f', 2) << ","
                << QString::fromStdString(tx.getDate()) << "\n";
        }
        file.close();
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Export Successful",
        "Bank statement successfully saved to:\n" + filePath + "\n\nDo you want to send it via email?",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        bool ok;
        QString inputEmail = QInputDialog::getText(
            this,
            "Send Statement via Email",
            "Enter recipient email address:",
            QLineEdit::Normal,
            "",
            &ok
        );

        if (ok && !inputEmail.trimmed().isEmpty()) {
            QString targetEmail = inputEmail.trimmed();
            if (sendStatementEmail(targetEmail.toStdString(), filePath.toStdString())) {
                QMessageBox::information(this, "Email Sent", "Bank statement has been sent to " + targetEmail + "!");
            } else {
                QMessageBox::warning(this, "Email Error", "Failed to send email. Please check your internet connection or .env SMTP settings.");
            }
        }
    }
}

bool CustomerWindow::sendStatementEmail(const std::string& toEmail, const std::string& filePath) {
    const char* envEmail = std::getenv("SMTP_EMAIL");
    const char* envPass = std::getenv("SMTP_PASSWORD");

    if (!envEmail || !envPass) {
        qWarning() << "SMTP_EMAIL or SMTP_PASSWORD not set in environment!";
        return false;
    }

    std::string senderEmail(envEmail);
    std::string appPassword(envPass);

    CURL* curl = curl_easy_init();
    if (!curl) return false;

    curl_easy_setopt(curl, CURLOPT_URL, "smtps://smtp.gmail.com:465");
    curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
    curl_easy_setopt(curl, CURLOPT_USERNAME, senderEmail.c_str());
    curl_easy_setopt(curl, CURLOPT_PASSWORD, appPassword.c_str());

    std::string mailFrom = "<" + senderEmail + ">";
    std::string mailTo = "<" + toEmail + ">";
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, mailFrom.c_str());

    struct curl_slist* recipients = nullptr;
    recipients = curl_slist_append(recipients, mailTo.c_str());
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

    struct curl_slist* headers = nullptr;
    std::string toHeader = "To: " + toEmail;
    std::string fromHeader = "From: Bank Management System <" + senderEmail + ">";
    headers = curl_slist_append(headers, toHeader.c_str());
    headers = curl_slist_append(headers, fromHeader.c_str());
    headers = curl_slist_append(headers, "Subject: Your Bank Account Statement");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_mime* mime = curl_mime_init(curl);

    curl_mimepart* part = curl_mime_addpart(mime);
    curl_mime_data(part, "Dear Customer,\n\nPlease find attached your requested bank account statement.\n\nBest regards,\nBank Management System", CURL_ZERO_TERMINATED);

    part = curl_mime_addpart(mime);
    curl_mime_filedata(part, filePath.c_str());
    curl_mime_encoder(part, "base64");

    curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

    CURLcode res = curl_easy_perform(curl);

    curl_slist_free_all(recipients);
    curl_slist_free_all(headers);
    curl_mime_free(mime);
    curl_easy_cleanup(curl);

    return (res == CURLE_OK);
}
