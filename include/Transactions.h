#pragma once
#include <string>

class Transaction {
private:
    int id;
    std::string type;
    double amount;
    std::string date;
    int account_id;

public:
    Transaction();
    Transaction(const std::string& type, double amount, int account_id);
    Transaction(int id, const std::string& type, double amount,
                const std::string& date, int account_id);

    int getId() const;
    std::string getType() const;
    double getAmount() const;
    std::string getDate() const;
    int getAccountId() const;

    void setId(int newId);
    void setType(const std::string& newType);
    void setAmount(double newAmount);
    void setDate(const std::string& newDate);
    void setAccountId(int newAccountId);

    void display() const;
};
