#include "BankService.h"
#include <iostream>

BankService::BankService(Database& database) : db(database) {}

bool BankService::deposit(const std::string& accountNumber, double amount) {
    if (amount <= 0) {
        return false;
    }

    Account acc = db.getAccountByNumber(accountNumber);
    if (acc.getAccountNumber().empty()) {
        return false;
    }
    if (!acc.isActive()) {
        std::cerr << "Account is not ACTIVE!\n";
        return false;
    }
    return db.deposit(accountNumber, amount);
}

bool BankService::withdraw(const std::string& accountNumber, double amount) {
    if (amount <= 0) {
        return false;
    }

    Account acc = db.getAccountByNumber(accountNumber);
    if (acc.getAccountNumber().empty()) {
        return false;
    }
    if (!acc.isActive()) {
        std::cerr << "Account is not ACTIVE!\n";
        return false;
    }
    if (acc.getBalance() < amount) {
        return false;
    }

    return db.withdraw(accountNumber, amount);
}

bool BankService::transfer(const std::string& fromAccount, const std::string& toAccount, double amount) {
    if (amount <= 0 || fromAccount == toAccount) {
        return false;
    }

    Account src = db.getAccountByNumber(fromAccount);
    Account dest = db.getAccountByNumber(toAccount);

    if (src.getAccountNumber().empty() || dest.getAccountNumber().empty()) {
        return false;
    }
    if (!src.isActive() || !dest.isActive()) {
        std::cerr << "One or both accounts are not ACTIVE!\n";
        return false;
    }
    if (src.getBalance() < amount) {
        return false;
    }

    return db.transfer(fromAccount, toAccount, amount);
}

bool BankService::registerCustomer(const Customer& customer) {
    return db.addCustomer(customer);
}

bool BankService::openAccount(const Account& account) {
    if (account.getAccountNumber().empty()) {
        return false;
    }
    return db.createAccount(account);
}

Account BankService::getAccountDetails(const std::string& accountNumber) {
    return db.getAccountByNumber(accountNumber);
}

std::vector<Transaction> BankService::getAccountHistory(const std::string& accountNumber) {
    Account acc = db.getAccountByNumber(accountNumber);
    if (acc.getAccountNumber().empty()) {
        return {};
    }
    return db.getTransactionsByAccount(acc.getId());
}

void BankService::printAccountDetails(const std::string& accountNumber) {
    Account acc = db.getAccountByNumber(accountNumber);
    if (acc.getAccountNumber().empty()) {
        std::cout << "Account not found.\n";
        return;
    }
    acc.display();
}

void BankService::printStatement(const std::string& accountNumber) {
    Account acc = db.getAccountByNumber(accountNumber);
    if (acc.getAccountNumber().empty()) {
        std::cout << "Account not found.\n";
        return;
    }

    std::vector<Transaction> history = db.getTransactionsByAccount(acc.getId());
    if (history.empty()) {
        std::cout << "No transactions found.\n";
        return;
    }

    for (const auto& txn : history) {
        txn.display();
    }
}

bool BankService::blockAccount(const std::string& accountNumber) {
    Account acc = db.getAccountByNumber(accountNumber);
    if (acc.getAccountNumber().empty()) {
        return false;
    }
    return db.updateAccountStatus(accountNumber, "BLOCKED");
}

bool BankService::activateAccount(const std::string& accountNumber) {
    Account acc = db.getAccountByNumber(accountNumber);
    if (acc.getAccountNumber().empty()) {
        return false;
    }
    return db.updateAccountStatus(accountNumber, "ACTIVE");
}

bool BankService::closeAccount(const std::string& accountNumber) {
    Account acc = db.getAccountByNumber(accountNumber);
    if (acc.getAccountNumber().empty()) {
        return false;
    }

    if (acc.getBalance() != 0.0) {
        std::cerr << "Cannot close account: Balance must be $0!\n";
        return false;
    }

    return db.updateAccountStatus(accountNumber, "CLOSED");
}
