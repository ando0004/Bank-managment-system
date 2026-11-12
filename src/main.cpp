#include <QApplication>
#include <iostream>
#include "Database.h"
#include "AuthService.h"
#include "LoginWindow.h"

int main(int argc, char *argv[]) {
    qputenv("QT_QPA_PLATFORM", "wayland;xcb");

    QApplication app(argc, argv);

    Database db;
    if (!db.connect()) {
        std::cerr << "Failed to connect to the database!\n";
        return 1;
    }
    db.initTables();

    AuthService auth(db);

    try {
        auth.registerUser("admin", "admin123", "admin@bank.local", "admin");
    } catch (...) {
    }

    try {
        auth.registerUser("user1", "user123", "user1@bank.local", "customer");
    } catch (...) {
    }

    LoginWindow loginWin(db, auth);
    loginWin.show();

    return app.exec();
}
