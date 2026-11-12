#include "PasswordUtils.h"

std::string PasswordUtils::hashPassword(const std::string& plainPassword) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(plainPassword.c_str()), plainPassword.length(), hash);

    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}

bool PasswordUtils::verifyPassword(const std::string& plainPassword, const std::string& hashedPassword) {
    return hashPassword(plainPassword) == hashedPassword;
}
