#pragma once
#include <string>

class User {
private:
    int id;
    std::string username;
    std::string password;
    std::string email;
    std::string role;
    int customerId;

public:
    User();
    User(const std::string& username, const std::string& password, const std::string& email, const std::string& role, int customerId = 0);
    User(int id, const std::string& username, const std::string& password, const std::string& email, const std::string& role, int customerId);

    int getId() const;
    std::string getUsername() const;
    std::string getPassword() const;
    std::string getEmail() const;
    std::string getRole() const;
    int getCustomerId() const;

    bool isAdmin() const;
    bool isCustomer() const;
};
