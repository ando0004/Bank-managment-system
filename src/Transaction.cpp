#include "Transactions.h"
#include <iostream>
#include<iomanip>
Transaction::Transaction()
    : id(0), type(""), amount(0.0), date(""), account_id(0) {}

Transaction::Transaction(const std::string& type, double amount, int account_id)
    : id(0), type(type), amount(amount), date(""), account_id(account_id) {}

Transaction::Transaction(int id, const std::string& type, double amount,
                         const std::string& date, int account_id)
    : id(id), type(type), amount(amount), date(date), account_id(account_id) {}

int Transaction::getId() const { return id; }
std::string Transaction::getType() const { return type; }
double Transaction::getAmount() const { return amount; }
std::string Transaction::getDate() const { return date; }
int Transaction::getAccountId() const { return account_id; }

void Transaction::setId(int newId) { id = newId; }
void Transaction::setType(const std::string& newType) { type = newType; }
void Transaction::setAmount(double newAmount) { amount = newAmount; }
void Transaction::setDate(const std::string& newDate) { date = newDate; }
void Transaction::setAccountId(int newAccountId) { account_id = newAccountId; }
void Transaction::display() const {
    std::cout << std::left
              << "Transaction ID: " << std::setw(5)  << id
              << " | Type: "        << std::setw(13) << type
              << " | Amount: "      << std::setw(10) << amount
              << " | Date: "        << std::setw(19) << date
              << " | Account ID: "  << account_id << std::endl;
}
