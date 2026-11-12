#include "User.h"

User::User()
    : id(0), username(""), password(""), email(""), role(""), customerId(0) {}

User::User(const std::string& username, const std::string& password, const std::string& email, const std::string& role, int customerId)
    : id(0), username(username), password(password), email(email), role(role), customerId(customerId) {}

User::User(int id, const std::string& username, const std::string& password, const std::string& email, const std::string& role, int customerId)
    : id(id), username(username), password(password), email(email), role(role), customerId(customerId) {}

int User::getId() const { return id; }
std::string User::getUsername() const { return username; }
std::string User::getPassword() const { return password; }
std::string User::getEmail() const { return email; }
std::string User::getRole() const { return role; }
int User::getCustomerId() const { return customerId; }

bool User::isAdmin() const { return role == "ADMIN" || role == "admin"; }
bool User::isCustomer() const { return role == "CUSTOMER" || role == "customer"; }
