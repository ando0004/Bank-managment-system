#include <gtest/gtest.h>
#include <openssl/sha.h>
#include <iomanip>
#include <sstream>
#include <string>

std::string computeSHA256(const std::string& input) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(input.c_str()), input.length(), hash);

    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}

TEST(CryptoTest, DeterministicHashVerification) {
    std::string password = "SecurePassword123!";
    std::string hash1 = computeSHA256(password);
    std::string hash2 = computeSHA256(password);

    EXPECT_EQ(hash1, hash2);
    EXPECT_EQ(hash1.length(), 64);
}

TEST(CryptoTest, DifferentPasswordsProduceDifferentHashes) {
    std::string pass1 = "Admin2026";
    std::string pass2 = "Admin2027";

    EXPECT_NE(computeSHA256(pass1), computeSHA256(pass2));
}

TEST(CryptoTest, EmptyStringKnownHash) {
    std::string empty = "";
    std::string expectedEmptyHash = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";

    EXPECT_EQ(computeSHA256(empty), expectedEmptyHash);
}

TEST(CryptoTest, SaltedPasswordChangesHash) {
    std::string password = "UserPass123";
    std::string salt1 = "random_salt_a";
    std::string salt2 = "random_salt_b";

    std::string hashWithSalt1 = computeSHA256(password + salt1);
    std::string hashWithSalt2 = computeSHA256(password + salt2);

    EXPECT_NE(hashWithSalt1, hashWithSalt2);
    EXPECT_NE(hashWithSalt1, computeSHA256(password));
}
