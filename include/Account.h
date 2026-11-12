#pragma once
#include <string>
#include <iostream>

class Account {
private:
    int id;
    int customerId;
    std::string accountNumber;
    double balance;
    std::string status;

public:
    Account();
    Account(int customerId, const std::string& accountNumber, double balance, const std::string& status = "ACTIVE");
    Account(int id, int customerId, const std::string& accountNumber, double balance, const std::string& status = "ACTIVE");

    int getId() const;
    int getCustomerId() const;
    std::string getAccountNumber() const;
    double getBalance() const;
    std::string getStatus() const { return status; }
    bool isActive() const { return status == "ACTIVE"; }

    void setId(int newId);
    void setCustomerId(int newCustomerId);
    void setAccountNumber(const std::string& newAccountNumber);
    void setBalance(double newBalance);
    void setStatus(const std::string& newStatus) { status = newStatus; }

    void display() const;
};
