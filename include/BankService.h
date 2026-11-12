#pragma once
#include "Database.h"
#include "Account.h"
#include "Customers.h"
#include "Transactions.h"
#include <string>
#include <vector>

class BankService {
private:
    Database& db;

public:
    BankService(Database& database);

    bool deposit(const std::string& accountNumber, double amount);
    bool withdraw(const std::string& accountNumber, double amount);
    bool transfer(const std::string& fromAccount, const std::string& toAccount, double amount);

    bool registerCustomer(const Customer& customer);
    bool openAccount(const Account& account);

    Account getAccountDetails(const std::string& accountNumber);
    std::vector<Transaction> getAccountHistory(const std::string& accountNumber);

    void printAccountDetails(const std::string& accountNumber);
    void printStatement(const std::string& accountNumber);

    bool blockAccount(const std::string& accountNumber);
    bool activateAccount(const std::string& accountNumber);
    bool closeAccount(const std::string& accountNumber);
};
