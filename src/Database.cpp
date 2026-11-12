#include "../include/Database.h"
#include <iostream>
#include<fstream>
static void loadEnv(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string val = line.substr(pos + 1);

            if (!val.empty() && (val.front() == '"' || val.front() == '\'')) {
                val = val.substr(1, val.length() - 2);
            }

            setenv(key.c_str(), val.c_str(), 1);
        }
    }
}
Database::Database() : driver(nullptr) {
    loadEnv(".env");
    loadEnv("../.env");

    const char* envHost = std::getenv("DB_HOST");
    const char* envUser = std::getenv("DB_USER");
    const char* envPass = std::getenv("DB_PASSWORD");
    const char* envDb   = std::getenv("DB_NAME");

    host     = envHost ? envHost : "tcp://127.0.0.1:3306";
    user     = envUser ? envUser : "root";
    password = envPass ? envPass : "";
    schema   = envDb   ? envDb   : "bank_management_system";
}

Database::Database(const std::string& h, const std::string& u,
                   const std::string& p, const std::string& db)
    : host(h), user(u), password(p), schema(db), driver(nullptr) {}
Database::~Database(){
    disconnect();
}
bool Database::connect(){
    try{
        driver = sql::mysql::get_driver_instance();
        con.reset(driver->connect(host,user,password));
        con->setSchema(schema);
        return true;
    }catch(sql::SQLException &e){
        std::cerr<< "Error connecting to database: " << e.what() << std::endl;
        return false;
    }
}
void Database::disconnect(){
    if(con){
        con->close();
        con.reset();
    }
}
sql::Connection* Database::getConnection(){
    return con.get();
}
bool Database::initTables(){
    if(!con) return false;
    try{
        std::unique_ptr<sql::Statement> stmt(con->createStatement());
        stmt->execute(
            "CREATE TABLE IF NOT EXISTS Customers("
            "id INT AUTO_INCREMENT PRIMARY KEY, "
            "name VARCHAR(100) NOT NULL, "
            "surname VARCHAR(100) NOT NULL, "
            "phone VARCHAR(20) NOT NULL, "
            "address VARCHAR(100));"
        );
            stmt->execute(
                "CREATE TABLE IF NOT EXISTS Accounts("
                "id INT AUTO_INCREMENT PRIMARY KEY, "
                "account_number VARCHAR(20) NOT NULL UNIQUE, "
                "customer_id INT NOT NULL, "
                "balance DECIMAL(15,2) NOT NULL DEFAULT 0, "
                "status VARCHAR(20) NOT NULL DEFAULT 'ACTIVE', "
                "FOREIGN KEY (customer_id) REFERENCES Customers(id));"
        );
        stmt->execute(
            "CREATE TABLE IF NOT EXISTS Transactions("
            "id INT AUTO_INCREMENT PRIMARY KEY, "
            "type VARCHAR(20) NOT NULL, "
            "amount DECIMAL(15,2) NOT NULL, "
            "date DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP, "
            "account_id INT NOT NULL, "
            "FOREIGN KEY (account_id) REFERENCES Accounts(id));"
        );
        stmt->execute(
            "CREATE TABLE IF NOT EXISTS Users ("
            "id INT AUTO_INCREMENT PRIMARY KEY, "
            "username VARCHAR(50) NOT NULL UNIQUE, "
            "password VARCHAR(255) NOT NULL, "
            "email VARCHAR(100) UNIQUE, "
            "role VARCHAR(20) NOT NULL, "
            "customer_id INT NULL, "
            "FOREIGN KEY (customer_id) REFERENCES Customers(id)"
            ");"
        );
        stmt->execute(
            "CREATE TABLE IF NOT EXISTS Loans ("
            "    id INT AUTO_INCREMENT PRIMARY KEY,"
            "    customer_id INT NOT NULL,"
            "    account_id INT NOT NULL,"
            "    amount DECIMAL(15, 2) NOT NULL,"
            "    remaining_amount DECIMAL(15, 2) NOT NULL,"
            "    monthly_payment DECIMAL(15, 2) NOT NULL,"
            "    term_months INT NOT NULL,"
            "    status VARCHAR(20) DEFAULT 'ACTIVE',"
            "    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
            "    FOREIGN KEY (customer_id) REFERENCES Customers(id) ON DELETE CASCADE,"
            "    FOREIGN KEY (account_id) REFERENCES Accounts(id) ON DELETE CASCADE"
            ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;"
        );
        return true;
    }catch(sql::SQLException &e){
        std::cerr<< "Error initializing tables: " << e.what() << std::endl;
        return false;
    }
}
bool Database::addCustomer(const Customer& customer) {
    if (!con) return false;

    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(
            con->prepareStatement("INSERT INTO Customers (name, surname, phone, address) VALUES (?, ?, ?, ?)")
        );

        pstmt->setString(1, customer.getName());
        pstmt->setString(2, customer.getSurname());
        pstmt->setString(3, customer.getPhone());
        pstmt->setString(4, customer.getAddress());

        pstmt->executeUpdate();
        return true;
    } catch (sql::SQLException& e) {
        std::cerr << "Error adding customer: " << e.what() << std::endl;
        return false;
    }
}
Customer Database::getCustomerById(int id) {
    if (!con) return Customer();

    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(
            con->prepareStatement("SELECT id, name, surname, phone, address FROM Customers WHERE id = ?")
        );
        pstmt->setInt(1, id);

        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        if (res->next()) {
            return Customer(
                res->getInt("id"),
                res->getString("name"),
                res->getString("surname"),
                res->getString("phone"),
                res->getString("address")
            );
        }
    } catch (sql::SQLException& e) {
        std::cerr << "Error getting customer: " << e.what() << std::endl;
    }

    return Customer();
}
bool Database::createAccount(const Account& account){
    if(!con) return false;
    try{
        std::unique_ptr<sql::PreparedStatement> pstmt(
            con->prepareStatement("INSERT INTO Accounts (account_number, customer_id, balance, status) VALUES (?, ?, ?, ?)")
        );
        pstmt->setString(1, account.getAccountNumber());
        pstmt->setInt(2, account.getCustomerId());
        pstmt->setDouble(3, account.getBalance());
        pstmt->setString(4, account.getStatus().empty() ? "ACTIVE" : account.getStatus());
        pstmt->executeUpdate();
        return true;
    } catch (sql::SQLException& e) {
        std::cerr << "Error creating account: " << e.what() << std::endl;
        return false;
    }
}
Account Database::getAccountByNumber(const std::string& accountNumber){

    if(!con) return Account();
    try{
        std::unique_ptr<sql::PreparedStatement> pstmt(
            con->prepareStatement("SELECT id, account_number, customer_id, balance , status FROM Accounts WHERE account_number = ?")
        );
        pstmt->setString(1,accountNumber);
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        if(res->next()){
            return Account(
                res->getInt("id"),
                res->getInt("customer_id"),
                res->getString("account_number"),
                res->getDouble("balance"),
                res->getString("status")
            );
        }
    }catch(sql::SQLException& e){
        std::cerr << "Error getting account: " << e.what() << std::endl;
    }
    return Account();
}
bool Database::deposit(const std::string& accountNumber, double amount){
    if(!con) return false;
    if (amount <= 0) {
        std::cerr << "Amount must be positive!" << std::endl;
        return false;
    }
    try{
        con->setAutoCommit(false);
        std::unique_ptr<sql::PreparedStatement> upd(
            con->prepareStatement("UPDATE Accounts SET balance = balance + ? WHERE account_number = ?")
        );
        upd->setDouble(1, amount);
        upd->setString(2, accountNumber);
        if(upd->executeUpdate() ==0){
            std::cerr<<"Account not found!"<<std::endl;
            con->rollback();
            con->setAutoCommit(true);
            return false;
        }
        std::unique_ptr<sql::PreparedStatement> ins(
            con->prepareStatement(
                "INSERT INTO Transactions (type, amount, account_id) "
                "SELECT 'deposit', ?, id FROM Accounts WHERE account_number = ?")
        );
        ins->setDouble(1, amount);
        ins->setString(2, accountNumber);
        ins->executeUpdate();
        con->commit();
        con->setAutoCommit(true);
        return true;
    }catch(sql::SQLException& e){
        std::cerr << "Error during deposit: " << e.what() << std::endl;
        try {
            con->rollback();
            con->setAutoCommit(true);
        } catch (sql::SQLException& rollbackEx) {
            std::cerr << "Error during rollback: " << rollbackEx.what() << std::endl;
        }
        return false;
    }

}
bool Database::withdraw(const std::string& accountNumber, double amount) {
    if (!con) return false;
    if (amount <= 0) {
        std::cerr << "Amount must be positive!" << std::endl;
        return false;
    }

    try {
        con->setAutoCommit(false);
        std::unique_ptr<sql::PreparedStatement> sel(
            con->prepareStatement(
                "SELECT balance FROM Accounts WHERE account_number = ? FOR UPDATE")
        );
        sel->setString(1, accountNumber);
        std::unique_ptr<sql::ResultSet> res(sel->executeQuery());

        if (!res->next()) {
            std::cerr << "Account not found!" << std::endl;
            con->rollback();
            con->setAutoCommit(true);
            return false;
        }

        if (res->getDouble("balance") < amount) {
            std::cerr << "Insufficient funds!" << std::endl;
            con->rollback();
            con->setAutoCommit(true);
            return false;
        }

        std::unique_ptr<sql::PreparedStatement> upd(
            con->prepareStatement(
                "UPDATE Accounts SET balance = balance - ? WHERE account_number = ?")
        );
        upd->setDouble(1, amount);
        upd->setString(2, accountNumber);
        upd->executeUpdate();

        std::unique_ptr<sql::PreparedStatement> ins(
            con->prepareStatement(
                "INSERT INTO Transactions (type, amount, account_id) "
                "SELECT 'withdraw', ?, id FROM Accounts WHERE account_number = ?")
        );
        ins->setDouble(1, amount);
        ins->setString(2, accountNumber);
        ins->executeUpdate();

        con->commit();
        con->setAutoCommit(true);
        return true;

    } catch (sql::SQLException& e) {
        std::cerr << "Error during withdraw: " << e.what() << std::endl;
        try { con->rollback(); } catch (...) {}
        try { con->setAutoCommit(true); } catch (...) {}
        return false;
    }
}
bool Database::transfer(const std::string& fromAccount, const std::string& toAccount, double amount) {
    if (!con) return false;
    if (amount <= 0) {
        std::cerr << "Amount must be positive!" << std::endl;
        return false;
    }
    if (fromAccount == toAccount) {
        std::cerr << "Cannot transfer to the same account!" << std::endl;
        return false;
    }

    try {
        con->setAutoCommit(false);

        std::unique_ptr<sql::PreparedStatement> sel(
            con->prepareStatement(
                "SELECT account_number, balance FROM Accounts "
                "WHERE account_number IN (?, ?) ORDER BY id FOR UPDATE")
        );
        sel->setString(1, fromAccount);
        sel->setString(2, toAccount);
        std::unique_ptr<sql::ResultSet> res(sel->executeQuery());

        int found = 0;
        double fromBalance = 0.0;
        while (res->next()) {
            ++found;
            std::string accNum = res->getString("account_number");
            if (accNum == fromAccount) {
                fromBalance = res->getDouble("balance");
            }
        }

        if (found != 2) {
            std::cerr << "One or both accounts not found!" << std::endl;
            con->rollback();
            con->setAutoCommit(true);
            return false;
        }

        if (fromBalance < amount) {
            std::cerr << "Insufficient funds!" << std::endl;
            con->rollback();
            con->setAutoCommit(true);
            return false;
        }

        std::unique_ptr<sql::PreparedStatement> dec(
            con->prepareStatement(
                "UPDATE Accounts SET balance = balance - ? WHERE account_number = ?")
        );
        dec->setDouble(1, amount);
        dec->setString(2, fromAccount);
        dec->executeUpdate();

        std::unique_ptr<sql::PreparedStatement> inc(
            con->prepareStatement(
                "UPDATE Accounts SET balance = balance + ? WHERE account_number = ?")
        );
        inc->setDouble(1, amount);
        inc->setString(2, toAccount);
        inc->executeUpdate();

        std::unique_ptr<sql::PreparedStatement> logOut(
            con->prepareStatement(
                "INSERT INTO Transactions (type, amount, account_id) "
                "SELECT 'transfer_out', ?, id FROM Accounts WHERE account_number = ?")
        );
        logOut->setDouble(1, amount);
        logOut->setString(2, fromAccount);
        logOut->executeUpdate();

        std::unique_ptr<sql::PreparedStatement> logIn(
            con->prepareStatement(
                "INSERT INTO Transactions (type, amount, account_id) "
                "SELECT 'transfer_in', ?, id FROM Accounts WHERE account_number = ?")
        );
        logIn->setDouble(1, amount);
        logIn->setString(2, toAccount);
        logIn->executeUpdate();

        con->commit();
        con->setAutoCommit(true);
        return true;

    } catch (sql::SQLException& e) {
        std::cerr << "Error during transfer: " << e.what() << std::endl;
        try { con->rollback(); } catch (...) {}
        try { con->setAutoCommit(true); } catch (...) {}
        return false;
    }
}
std::vector<Transaction> Database::getTransactionsByAccount(int accountId) {
    std::vector<Transaction> list;
    if (!con) return list;
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(
            con->prepareStatement(
                "SELECT id, type, amount, date, account_id FROM Transactions "
                "WHERE account_id = ? ORDER BY date DESC")
        );
        pstmt->setInt(1, accountId);
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        while (res->next()) {
            list.push_back(Transaction(
                res->getInt("id"),
                res->getString("type"),
                res->getDouble("amount"),
                res->getString("date"),
                res->getInt("account_id")));
        }
    } catch (sql::SQLException& e) {
        std::cerr << "Error getting transactions: " << e.what() << std::endl;
    }
    return list;
}

bool Database::updateAccountStatus(const std::string& accountNumber, const std::string& newStatus) {
    try {
        std::unique_ptr<sql::PreparedStatement> stmt(
            con->prepareStatement("UPDATE Accounts SET status = ? WHERE account_number = ?")
        );
        stmt->setString(1, newStatus);
        stmt->setString(2, accountNumber);

        int rowsUpdated = stmt->executeUpdate();
        return rowsUpdated > 0;
    } catch (sql::SQLException& e) {
        std::cerr << "Error updating account status: " << e.what() << "\n";
        return false;
    }
}

bool Database::addUser(const User& user) {
    if (!con) return false;

    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(
            con->prepareStatement("INSERT INTO Users (username, password, email, role, customer_id) VALUES (?, ?, ?, ?, ?)")
        );

        pstmt->setString(1, user.getUsername());
        pstmt->setString(2, user.getPassword());
        pstmt->setString(3, user.getEmail());
        pstmt->setString(4, user.getRole());
        if (user.getCustomerId() > 0) {
            pstmt->setInt(5, user.getCustomerId());
        } else {
            pstmt->setNull(5, sql::DataType::INTEGER);
        }

        pstmt->executeUpdate();
        return true;
    } catch (sql::SQLException& e) {
        std::cerr << "Error adding user: " << e.what() << std::endl;
        return false;
    }
}

User Database::getUserByUsername(const std::string& username) {
    if (!con) return User();

    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(
            con->prepareStatement("SELECT id, username, password, email, role, customer_id FROM Users WHERE username = ?")
        );
        pstmt->setString(1, username);

        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        if (res->next()) {
            return User(
                res->getInt("id"),
                res->getString("username"),
                res->getString("password"),
                res->getString("email"),
                res->getString("role"),
                res->isNull("customer_id") ? 0 : res->getInt("customer_id")
            );
        }
    } catch (sql::SQLException& e) {
        std::cerr << "Error getting user: " << e.what() << std::endl;
    }

    return User();
}

std::vector<Transaction> Database::getTransactionHistory(const std::string& accountNumber) {
    std::vector<Transaction> history;
    if (!con) return history;

    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(
            con->prepareStatement(
                "SELECT t.id, t.type, t.amount, t.date, t.account_id "
                "FROM Transactions t "
                "JOIN Accounts a ON t.account_id = a.id "
                "WHERE a.account_number = ? "
                "ORDER BY t.date DESC"
            )
        );
        pstmt->setString(1, accountNumber);

        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        while (res->next()) {
            Transaction tx(
                res->getInt("id"),
                res->getString("type"),
                res->getDouble("amount"),
                res->getString("date"),
                res->getInt("account_id")
            );
            history.push_back(tx);
        }
    } catch (sql::SQLException& e) {
        std::cerr << "SQL Error in getTransactionHistory: " << e.what() << std::endl;
    }
    return history;
}

std::string Database::getAccountNumberByCustomerId(int customerId) {
    if (!con) return "";
    try {
        std::unique_ptr<sql::PreparedStatement> stmt(
            con->prepareStatement("SELECT account_number FROM Accounts WHERE customer_id = ? LIMIT 1")
        );
        stmt->setInt(1, customerId);
        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
        if (res->next()) {
            return res->getString("account_number");
        }
    } catch (sql::SQLException& e) {
        std::cerr << "Error getting account: " << e.what() << std::endl;
    }
    return "";
}

bool Database::applyAndDisburseLoan(int customerId, const std::string& accountNumber,
                                    double amount, int termMonths, double interestRate) {
    if (!con || amount <= 0 || termMonths <= 0) return false;

    try {
        con->setAutoCommit(false);

        Account acc = getAccountByNumber(accountNumber);
        if (acc.getId() == 0) {
            con->rollback();
            con->setAutoCommit(true);
            return false;
        }

        double totalInterest = amount * (interestRate / 100.0) * (termMonths / 12.0);
        double totalRepayment = amount + totalInterest;
        double monthlyPayment = totalRepayment / termMonths;

        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement(
            "INSERT INTO Loans (customer_id, account_id, amount, remaining_amount, "
            "monthly_payment, term_months, status) VALUES (?, ?, ?, ?, ?, ?, 'ACTIVE')"
        ));
        pstmt->setInt(1, customerId);
        pstmt->setInt(2, acc.getId());
        pstmt->setDouble(3, amount);
        pstmt->setDouble(4, totalRepayment);
        pstmt->setDouble(5, monthlyPayment);
        pstmt->setInt(6, termMonths);
        pstmt->executeUpdate();

        std::unique_ptr<sql::PreparedStatement> updateAcc(con->prepareStatement(
            "UPDATE Accounts SET balance = balance + ? WHERE account_number = ?"
        ));
        updateAcc->setDouble(1, amount);
        updateAcc->setString(2, accountNumber);
        updateAcc->executeUpdate();

        std::unique_ptr<sql::PreparedStatement> txStmt(con->prepareStatement(
            "INSERT INTO Transactions (account_id, type, amount, date) VALUES (?, 'LOAN_DISBURSEMENT', ?, NOW())"
        ));
        txStmt->setInt(1, acc.getId());
        txStmt->setDouble(2, amount);
        txStmt->executeUpdate();

        con->commit();
        con->setAutoCommit(true);
        return true;

    } catch (sql::SQLException& e) {
        con->rollback();
        con->setAutoCommit(true);
        std::cerr << "applyAndDisburseLoan error: " << e.what() << std::endl;
        return false;
    }
}

bool Database::hasActiveLoan(int customerId) {
    if (!con) return false;

    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement(
            "SELECT COUNT(*) AS active_count FROM Loans WHERE customer_id = ? AND status = 'ACTIVE'"
        ));
        pstmt->setInt(1, customerId);
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        if (res->next()) {
            return res->getInt("active_count") > 0;
        }
    } catch (sql::SQLException& e) {
        std::cerr << "hasActiveLoan error: " << e.what() << std::endl;
    }
    return false;
}

double Database::getTotalActiveLoanDebt(int customerId) {
    if (!con) return 0.0;

    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement(
            "SELECT COALESCE(SUM(remaining_amount), 0.0) AS total_debt FROM Loans "
            "WHERE customer_id = ? AND status = 'ACTIVE'"
        ));
        pstmt->setInt(1, customerId);
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        if (res->next()) {
            return res->getDouble("total_debt");
        }
    } catch (sql::SQLException& e) {
        std::cerr << "getTotalActiveLoanDebt error: " << e.what() << std::endl;
    }
    return 0.0;
}

std::vector<LoanInfo> Database::getCustomerLoans(int customerId) {
    std::vector<LoanInfo> loans;
    if (!con) return loans;

    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement(
            "SELECT id, customer_id, account_id, amount, remaining_amount, "
            "monthly_payment, term_months, status, created_at "
            "FROM Loans "
            "WHERE customer_id = ? "
            "ORDER BY created_at DESC"
        ));
        pstmt->setInt(1, customerId);

        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        while (res->next()) {
            LoanInfo loan;
            loan.id = res->getInt("id");
            loan.customerId = res->getInt("customer_id");
            loan.accountId = res->getInt("account_id");
            loan.amount = res->getDouble("amount");
            loan.remainingAmount = res->getDouble("remaining_amount");
            loan.monthlyPayment = res->getDouble("monthly_payment");
            loan.termMonths = res->getInt("term_months");
            loan.status = res->getString("status");
            loan.createdAt = res->getString("created_at");
            loans.push_back(loan);
        }
    } catch (sql::SQLException& e) {
        std::cerr << "getCustomerLoans error: " << e.what() << std::endl;
    }
    return loans;
}

bool Database::payLoan(int loanId, int customerId, double amount) {
    if (amount <= 0.0) return false;

    try {
        std::unique_ptr<sql::PreparedStatement> checkBalance(
            con->prepareStatement("SELECT balance FROM Accounts WHERE customer_id = ?")
        );
        checkBalance->setInt(1, customerId);
        std::unique_ptr<sql::ResultSet> resBal(checkBalance->executeQuery());
        if (!resBal->next()) return false;

        double currentBalance = resBal->getDouble("balance");
        if (currentBalance < amount) return false;

        std::unique_ptr<sql::PreparedStatement> checkLoan(
            con->prepareStatement(
                "SELECT remaining_amount, status, monthly_payment "
                "FROM Loans WHERE id = ? AND customer_id = ?"
            )
        );
        checkLoan->setInt(1, loanId);
        checkLoan->setInt(2, customerId);
        std::unique_ptr<sql::ResultSet> resLoan(checkLoan->executeQuery());
        if (!resLoan->next()) return false;

        std::string status = resLoan->getString("status");
        if (status != "APPROVED" && status != "ACTIVE") return false;

        double remainingLoan = resLoan->getDouble("remaining_amount");
        double monthlyPayment = resLoan->getDouble("monthly_payment");

        if (amount > remainingLoan) {
            amount = remainingLoan;
        }

        con->setAutoCommit(false);

        std::unique_ptr<sql::PreparedStatement> updateAcc(
            con->prepareStatement("UPDATE Accounts SET balance = balance - ? WHERE customer_id = ?")
        );
        updateAcc->setDouble(1, amount);
        updateAcc->setInt(2, customerId);
        updateAcc->executeUpdate();

        double newRemainingAmount = remainingLoan - amount;
        std::string newStatus = status;
        double newMonthlyPayment = monthlyPayment;

        if (newRemainingAmount <= 0.01) {
            newRemainingAmount = 0.0;
            newStatus = "PAID";
            newMonthlyPayment = 0.0;
        } else if (newRemainingAmount < monthlyPayment) {
            newMonthlyPayment = newRemainingAmount;
        }

        std::unique_ptr<sql::PreparedStatement> updateLoan(
            con->prepareStatement(
                "UPDATE Loans SET remaining_amount = ?, status = ?, monthly_payment = ? WHERE id = ?"
            )
        );
        updateLoan->setDouble(1, newRemainingAmount);
        updateLoan->setString(2, newStatus);
        updateLoan->setDouble(3, newMonthlyPayment);
        updateLoan->setInt(4, loanId);
        updateLoan->executeUpdate();

        int accId = 0;
        std::unique_ptr<sql::PreparedStatement> getAcc(
            con->prepareStatement("SELECT id FROM Accounts WHERE customer_id = ? LIMIT 1")
        );
        getAcc->setInt(1, customerId);
        std::unique_ptr<sql::ResultSet> resAcc(getAcc->executeQuery());
        if (resAcc->next()) {
            accId = resAcc->getInt("id");
        }
        if (accId > 0) {
            std::unique_ptr<sql::PreparedStatement> insertTx(
                con->prepareStatement(
                    "INSERT INTO Transactions (type, amount, date, account_id) "
                    "VALUES ('LOAN_PAYMENT', ?, NOW(), ?)"
                )
            );
            insertTx->setDouble(1, amount);
            insertTx->setInt(2, accId);
            insertTx->executeUpdate();
        }
        con->commit();
        con->setAutoCommit(true);
        return true;
    } catch (sql::SQLException &e) {
        con->rollback();
        con->setAutoCommit(true);
        return false;
    }
}

User Database::getUserByEmail(const std::string& email) {
    if (!con) return User();

    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(
            con->prepareStatement("SELECT id, username, password, email, role, customer_id FROM Users WHERE email = ?")
        );
        pstmt->setString(1, email);

        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        if (res->next()) {
            return User(
                res->getInt("id"),
                res->getString("username"),
                res->getString("password"),
                res->getString("email"),
                res->getString("role"),
                res->getInt("customer_id")
            );
        }
    } catch (sql::SQLException& e) {
        std::cerr << "Error fetching user by email: " << e.what() << std::endl;
    }
    return User();
}

bool Database::updateUserPassword(const std::string& email, const std::string& newHashedPassword) {
    if (!con) return false;

    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(
            con->prepareStatement("UPDATE Users SET password = ? WHERE email = ?")
        );
        pstmt->setString(1, newHashedPassword);
        pstmt->setString(2, email);

        return pstmt->executeUpdate() > 0;
    } catch (sql::SQLException& e) {
        std::cerr << "Error updating password: " << e.what() << std::endl;
        return false;
    }
}

bool Database::registerCustomerWithAccount(const User& user, const std::string& fullName, const std::string& phone) {
    if (!con) return false;

    try {
        con->setAutoCommit(false);

        std::unique_ptr<sql::PreparedStatement> custStmt(
            con->prepareStatement("INSERT INTO Customers (name, surname, phone, address) VALUES (?, ?, ?, ?)")
        );
        custStmt->setString(1, fullName.empty() ? user.getUsername() : fullName);
        custStmt->setString(2, "Client");
        custStmt->setString(3, phone.empty() ? "+37400000000" : phone);
        custStmt->setString(4, "Yerevan, Armenia");
        custStmt->executeUpdate();

        std::unique_ptr<sql::Statement> idStmt(con->createStatement());
        std::unique_ptr<sql::ResultSet> idRes(idStmt->executeQuery("SELECT LAST_INSERT_ID() AS id"));
        int newCustomerId = 0;
        if (idRes->next()) {
            newCustomerId = idRes->getInt("id");
        }

        char accBuf[32];
        snprintf(accBuf, sizeof(accBuf), "AM01BANK%04d", newCustomerId);
        std::string newAccNum(accBuf);

        std::unique_ptr<sql::PreparedStatement> accStmt(
            con->prepareStatement("INSERT INTO Accounts (account_number, customer_id, balance, status) VALUES (?, ?, 0.00, '\''ACTIVE'\'')")
        );
        accStmt->setString(1, newAccNum);
        accStmt->setInt(2, newCustomerId);
        accStmt->executeUpdate();

        std::unique_ptr<sql::PreparedStatement> userStmt(
            con->prepareStatement("INSERT INTO Users (username, password, email, role, customer_id) VALUES (?, ?, ?, ?, ?)")
        );
        userStmt->setString(1, user.getUsername());
        userStmt->setString(2, user.getPassword());
        userStmt->setString(3, user.getEmail());
        userStmt->setString(4, user.getRole());
        userStmt->setInt(5, newCustomerId);
        userStmt->executeUpdate();

        con->commit();
        con->setAutoCommit(true);
        return true;
    } catch (sql::SQLException& e) {
        con->rollback();
        con->setAutoCommit(true);
        std::cerr << "Error in registerCustomerWithAccount: " << e.what() << std::endl;
        return false;
    }
}
