#include "AuthService.h"
#include "PasswordUtils.h"
#include <stdexcept>

User AuthService::login(const std::string& username, const std::string& password) {
    User user = db.getUserByUsername(username);

    if (user.getUsername().empty() || !PasswordUtils::verifyPassword(password, user.getPassword())) {
        throw std::runtime_error("Invalid username or password");
    }
    return user;
}

bool AuthService::registerUser(const std::string& username, const std::string& password, const std::string& email, const std::string& role, int customerId) {
    if (username.empty() || password.empty() || email.empty()) {
        throw std::runtime_error("Username, password, and email are required!");
    }

    if (email.find('@') == std::string::npos || email.find('.') == std::string::npos) {
        throw std::runtime_error("Invalid email format!");
    }

    if (!db.getUserByUsername(username).getUsername().empty()) {
        return false;
    }

    if (!db.getUserByEmail(email).getUsername().empty()) {
        throw std::runtime_error("This email is already registered!");
    }

    std::string hashedPassword = PasswordUtils::hashPassword(password);
    User newUser(username, hashedPassword, email, role, customerId);

    if (role == "customer" && customerId == 0) {
        return db.registerCustomerWithAccount(newUser, username, "+37400000000");
    }

    return db.addUser(newUser);
}

bool AuthService::resetPasswordByEmail(const std::string& email, const std::string& newPassword) {
    if (db.getUserByEmail(email).getUsername().empty()) {
        return false;
    }
    std::string hashedPassword = PasswordUtils::hashPassword(newPassword);
    return db.updateUserPassword(email, hashedPassword);
}
