#include "LoginWindow.h"
#include "MainWindow.h"
#include "CustomerWindow.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QDialog>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QRandomGenerator>
#include <curl/curl.h>
#include <cstdlib>

LoginWindow::LoginWindow(Database& database, AuthService& authService, QWidget *parent)
    : QWidget(parent), db(database), auth(authService), mainWindow(nullptr) {

    setWindowTitle("Bank Management System - Login");
    setFixedSize(360, 320);

    QVBoxLayout* layout = new QVBoxLayout(this);

    QLabel* titleLabel = new QLabel("<h3>Welcome to Bank System</h3>", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);

    usernameInput = new QLineEdit(this);
    usernameInput->setPlaceholderText("Username");
    layout->addWidget(usernameInput);

    passwordInput = new QLineEdit(this);
    passwordInput->setPlaceholderText("Password");
    passwordInput->setEchoMode(QLineEdit::Password);
    layout->addWidget(passwordInput);

    loginBtn = new QPushButton("Login", this);
    loginBtn->setStyleSheet("font-weight: bold; padding: 6px;");
    layout->addWidget(loginBtn);

    registerBtn = new QPushButton("Create New Account", this);
    registerBtn->setStyleSheet("background-color: #2e7d32; color: white; font-weight: bold; padding: 6px;");
    layout->addWidget(registerBtn);

    forgotPassBtn = new QPushButton("Forgot Password?", this);
    forgotPassBtn->setStyleSheet("QPushButton { color: #1976d2; border: none; background: transparent; text-decoration: underline; }");
    forgotPassBtn->setCursor(Qt::PointingHandCursor);
    layout->addWidget(forgotPassBtn);

    statusLabel = new QLabel("", this);
    statusLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(statusLabel);

    connect(loginBtn, &QPushButton::clicked, this, &LoginWindow::handleLogin);
    connect(passwordInput, &QLineEdit::returnPressed, this, &LoginWindow::handleLogin);
    connect(registerBtn, &QPushButton::clicked, this, &LoginWindow::handleRegister);
    connect(forgotPassBtn, &QPushButton::clicked, this, &LoginWindow::handleForgotPassword);
}

LoginWindow::~LoginWindow() {
}

void LoginWindow::handleLogin() {
    std::string user = usernameInput->text().toStdString();
    std::string pass = passwordInput->text().toStdString();

    if (user.empty() || pass.empty()) {
        statusLabel->setStyleSheet("color: red;");
        statusLabel->setText("Please enter username and password");
        return;
    }

    try {
        User loggedUser = auth.login(user, pass);
        this->hide();

        if (loggedUser.getRole() == "admin") {
            MainWindow* mainWin = new MainWindow(db, loggedUser);
            mainWin->setAttribute(Qt::WA_DeleteOnClose);
            mainWin->show();
        } else {
            CustomerWindow* custWin = new CustomerWindow(db, loggedUser);
            custWin->setAttribute(Qt::WA_DeleteOnClose);
            custWin->show();
        }

    } catch (const std::exception& e) {
        statusLabel->setStyleSheet("color: red;");
        statusLabel->setText(e.what());
    }
}

void LoginWindow::handleRegister() {
    QDialog dialog(this);
    dialog.setWindowTitle("Register New Account");
    dialog.setMinimumWidth(320);

    QFormLayout form(&dialog);

    QLineEdit* regUser = new QLineEdit(&dialog);
    regUser->setPlaceholderText("Choose a username");
    form.addRow("Username *:", regUser);

    QLineEdit* regEmail = new QLineEdit(&dialog);
    regEmail->setPlaceholderText("example@gmail.com");
    form.addRow("Email *:", regEmail);

    QLineEdit* regPass = new QLineEdit(&dialog);
    regPass->setPlaceholderText("Min 4 characters");
    regPass->setEchoMode(QLineEdit::Password);
    form.addRow("Password *:", regPass);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);

    connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) return;

    std::string username = regUser->text().trimmed().toStdString();
    std::string email = regEmail->text().trimmed().toStdString();
    std::string password = regPass->text().toStdString();

    if (username.empty() || email.empty() || password.length() < 4) {
        QMessageBox::warning(this, "Validation Error", "All fields are required (Password must be at least 4 characters).");
        return;
    }

    try {
        if (auth.registerUser(username, password, email, "customer", 0)) {
            QMessageBox::information(this, "Success", "Account created successfully! You can now log in.");
            usernameInput->setText(QString::fromStdString(username));
            passwordInput->clear();
        } else {
            QMessageBox::warning(this, "Error", "Username already exists!");
        }
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Registration Failed", e.what());
    }
}

void LoginWindow::handleForgotPassword() {
    bool ok;
    QString emailInput = QInputDialog::getText(
        this,
        "Password Recovery",
        "Enter your registered email address:",
        QLineEdit::Normal,
        "",
        &ok
    );

    if (!ok || emailInput.trimmed().isEmpty()) return;
    std::string email = emailInput.trimmed().toStdString();

    User foundUser = db.getUserByEmail(email);
    if (foundUser.getUsername().empty()) {
        QMessageBox::warning(this, "Error", "No account found with this email address.");
        return;
    }

    int codeNum = QRandomGenerator::global()->bounded(100000, 999999);
    QString otpCode = QString::number(codeNum);

    if (!sendOtpEmail(email, otpCode.toStdString())) {
        QMessageBox::critical(this, "Email Error", "Failed to send verification code. Check your internet or .env SMTP settings.");
        return;
    }

    QString enteredCode = QInputDialog::getText(
        this,
        "Verify Code",
        "A 6-digit verification code has been sent to " + emailInput + ".\nEnter the code:",
        QLineEdit::Normal,
        "",
        &ok
    );

    if (!ok || enteredCode.trimmed() != otpCode) {
        QMessageBox::warning(this, "Invalid Code", "The verification code is incorrect.");
        return;
    }

    QString newPass = QInputDialog::getText(
        this,
        "Reset Password",
        "Enter new password for user '" + QString::fromStdString(foundUser.getUsername()) + "':",
        QLineEdit::Password,
        "",
        &ok
    );

    if (!ok || newPass.length() < 4) {
        QMessageBox::warning(this, "Weak Password", "Password must be at least 4 characters long.");
        return;
    }

    if (auth.resetPasswordByEmail(email, newPass.toStdString())) {
        QMessageBox::information(this, "Success", "Your password has been successfully updated! You can now log in.");
    } else {
        QMessageBox::critical(this, "Error", "Failed to update password in database.");
    }
}

bool LoginWindow::sendOtpEmail(const std::string& toEmail, const std::string& otpCode) {
    const char* envEmail = std::getenv("SMTP_EMAIL");
    const char* envPass = std::getenv("SMTP_PASSWORD");

    if (!envEmail || !envPass) return false;

    std::string senderEmail(envEmail);
    std::string appPassword(envPass);

    CURL* curl = curl_easy_init();
    if (!curl) return false;

    curl_easy_setopt(curl, CURLOPT_URL, "smtps://smtp.gmail.com:465");
    curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
    curl_easy_setopt(curl, CURLOPT_USERNAME, senderEmail.c_str());
    curl_easy_setopt(curl, CURLOPT_PASSWORD, appPassword.c_str());

    std::string mailFrom = "<" + senderEmail + ">";
    std::string mailTo = "<" + toEmail + ">";
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, mailFrom.c_str());

    struct curl_slist* recipients = nullptr;
    recipients = curl_slist_append(recipients, mailTo.c_str());
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

    struct curl_slist* headers = nullptr;
    std::string toHeader = "To: " + toEmail;
    std::string fromHeader = "From: Bank Security <" + senderEmail + ">";
    headers = curl_slist_append(headers, toHeader.c_str());
    headers = curl_slist_append(headers, fromHeader.c_str());
    headers = curl_slist_append(headers, "Subject: Password Reset Verification Code");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    std::string body = "Hello,\n\nYour password reset verification code is: " + otpCode +
                       "\n\nIf you did not request this, please ignore this email.\n\nBank Management System";

    curl_mime* mime = curl_mime_init(curl);
    curl_mimepart* part = curl_mime_addpart(mime);
    curl_mime_data(part, body.c_str(), CURL_ZERO_TERMINATED);
    curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

    CURLcode res = curl_easy_perform(curl);

    curl_slist_free_all(recipients);
    curl_slist_free_all(headers);
    curl_mime_free(mime);
    curl_easy_cleanup(curl);

    return (res == CURLE_OK);
}
