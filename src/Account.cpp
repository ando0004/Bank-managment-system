#include "Account.h"

#include "Account.h"

Account::Account()
    : id(0), customerId(0), accountNumber(""), balance(0.0), status("ACTIVE") {}

Account::Account(int customerId, const std::string& accountNumber, double balance, const std::string& status)
    : id(0), customerId(customerId), accountNumber(accountNumber), balance(balance), status(status) {}

Account::Account(int id, int customerId, const std::string& accountNumber, double balance, const std::string& status)
    : id(id), customerId(customerId), accountNumber(accountNumber), balance(balance), status(status) {}

int Account::getId() const {
    return id;
}

int Account::getCustomerId() const {
    return customerId;
}

std::string Account::getAccountNumber() const {
    return accountNumber;
}

double Account::getBalance() const {
    return balance;
}

void Account::setId(int newId) {
    id = newId;
}

void Account::setCustomerId(int newCustomerId) {
    customerId = newCustomerId;
}

void Account::setAccountNumber(const std::string& newAccountNumber) {
    accountNumber = newAccountNumber;
}

void Account::setBalance(double newBalance) {
    balance = newBalance;
}

void Account::display() const {
    std::cout << "Account ID: " << id
              << " | Customer ID: " << customerId
              << " | Account No: " << accountNumber
              << " | Balance: $" << balance << std::endl;
}
