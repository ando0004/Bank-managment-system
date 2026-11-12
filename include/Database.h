#pragma once
#include "Account.h"
#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include "Customers.h"
#include "Transactions.h"
#include "User.h"

struct LoanInfo {
    int id;
    int customerId;
    int accountId;
    double amount;
    double remainingAmount;
    double monthlyPayment;
    int termMonths;
    std::string status;
    std::string createdAt;
};

class Database {
    sql::mysql::MySQL_Driver *driver;
    std::unique_ptr<sql::Connection> con;
    std::string host;
    std::string user;
    std::string password;
    std::string schema;

public:
    Database();
    Database(const std::string& h, const std::string& u,
             const std::string& p, const std::string& db);
    ~Database();

    bool connect();
    void disconnect();
    bool initTables();
    sql::Connection* getConnection();

    bool addCustomer(const Customer& customer);
    Customer getCustomerById(int id);

    bool createAccount(const Account& account);
    Account getAccountByNumber(const std::string& accountNumber);

    bool deposit(const std::string& accountNumber, double amount);
    bool withdraw(const std::string& accountNumber, double amount);
    bool transfer(const std::string& fromAccount, const std::string& toAccount, double amount);

    std::vector<Transaction> getTransactionsByAccount(int accountId);
    bool updateAccountStatus(const std::string& accountNumber, const std::string& newStatus);

    bool addUser(const User& user);
    bool registerCustomerWithAccount(const User& user, const std::string& fullName, const std::string& phone);
    User getUserByUsername(const std::string& username);
    User getUserByEmail(const std::string& email);
    bool updateUserPassword(const std::string& email, const std::string& newHashedPassword);

    std::string getAccountNumberByCustomerId(int customerId);
    std::vector<Transaction> getTransactionHistory(const std::string& accountNumber);

    bool hasActiveLoan(int customerId);
    double getTotalActiveLoanDebt(int customerId);
    std::vector<LoanInfo> getCustomerLoans(int customerId);
    bool applyAndDisburseLoan(int customerId, const std::string& accountNumber,
                              double amount, int termMonths, double interestRate);
    bool payLoan(int loanId, int customerId, double amount);
};
