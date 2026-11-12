#include <gtest/gtest.h>
#include <string>

class BankAccount {
private:
    std::string accountNumber;
    double balance;

public:
    BankAccount(const std::string& accNum, double initialBalance)
        : accountNumber(accNum), balance(initialBalance) {}

    double getBalance() const { return balance; }
    std::string getAccountNumber() const { return accountNumber; }

    bool deposit(double amount) {
        if (amount <= 0.0) {
            return false;
        }
        balance += amount;
        return true;
    }

    bool withdraw(double amount) {
        if (amount <= 0.0 || amount > balance) {
            return false;
        }
        balance -= amount;
        return true;
    }

    bool transferTo(BankAccount& target, double amount) {
        if (accountNumber == target.getAccountNumber()) {
            return false;
        }
        if (amount <= 0.0 || amount > balance) {
            return false;
        }
        balance -= amount;
        target.balance += amount;
        return true;
    }
};

class AccountOperationsTest : public ::testing::Test {
protected:
    BankAccount* acc1;
    BankAccount* acc2;

    void SetUp() override {
        acc1 = new BankAccount("AM01BANK0001", 100000.0);
        acc2 = new BankAccount("AM01BANK0002", 50000.0);
    }

    void TearDown() override {
        delete acc1;
        delete acc2;
    }
};

TEST_F(AccountOperationsTest, DepositIncreasesBalance) {
    EXPECT_TRUE(acc1->deposit(50000.0));
    EXPECT_DOUBLE_EQ(acc1->getBalance(), 150000.0);
}

TEST_F(AccountOperationsTest, InvalidDepositRejected) {
    EXPECT_FALSE(acc1->deposit(0.0));
    EXPECT_FALSE(acc1->deposit(-5000.0));
    EXPECT_DOUBLE_EQ(acc1->getBalance(), 100000.0);
}

TEST_F(AccountOperationsTest, WithdrawDecreasesBalance) {
    EXPECT_TRUE(acc1->withdraw(40000.0));
    EXPECT_DOUBLE_EQ(acc1->getBalance(), 60000.0);
}

TEST_F(AccountOperationsTest, WithdrawFailsOnInsufficientFunds) {
    EXPECT_FALSE(acc1->withdraw(150000.0));
    EXPECT_DOUBLE_EQ(acc1->getBalance(), 100000.0);
}

TEST_F(AccountOperationsTest, TransferMovesMoneyCorrectly) {
    EXPECT_TRUE(acc1->transferTo(*acc2, 30000.0));
    EXPECT_DOUBLE_EQ(acc1->getBalance(), 70000.0);
    EXPECT_DOUBLE_EQ(acc2->getBalance(), 80000.0);
}

TEST_F(AccountOperationsTest, TransferFailsWhenBalanceTooLow) {
    EXPECT_FALSE(acc1->transferTo(*acc2, 200000.0));
    EXPECT_DOUBLE_EQ(acc1->getBalance(), 100000.0);
    EXPECT_DOUBLE_EQ(acc2->getBalance(), 50000.0);
}

TEST_F(AccountOperationsTest, TransferToSelfIsRejected) {
    EXPECT_FALSE(acc1->transferTo(*acc1, 10000.0));
    EXPECT_DOUBLE_EQ(acc1->getBalance(), 100000.0);
}
