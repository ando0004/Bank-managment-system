#pragma once
#include <string>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>

class PasswordUtils {
public:
    static std::string hashPassword(const std::string& plainPassword);
    static bool verifyPassword(const std::string& plainPassword, const std::string& hashedPassword);
};
