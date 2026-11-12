#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include "Database.h"
#include "AuthService.h"

class MainWindow;

class LoginWindow : public QWidget {
    Q_OBJECT

public:
    LoginWindow(Database& database, AuthService& authService, QWidget *parent = nullptr);
    ~LoginWindow();

private slots:
    void handleLogin();
    void handleRegister();
    void handleForgotPassword();

private:
    Database& db;
    AuthService& auth;

    QLineEdit* usernameInput;
    QLineEdit* passwordInput;
    QPushButton* loginBtn;
    QPushButton* registerBtn;
    QPushButton* forgotPassBtn;
    QLabel* statusLabel;

    MainWindow* mainWindow;
    bool sendOtpEmail(const std::string& toEmail, const std::string& otpCode);
};
