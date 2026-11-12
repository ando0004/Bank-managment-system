#pragma once
#include "Database.h"
#include "User.h"

class AuthService {
private:
    Database& db;

public:
    AuthService(Database& database) : db(database) {}

    User login(const std::string& username, const std::string& password);
    bool registerUser(const std::string& username, const std::string& password, const std::string& email, const std::string& role, int customerId = 0);
    bool resetPasswordByEmail(const std::string& email, const std::string& newPassword);
};
