# Bank Management System

Desktop banking application built with **C++**, **Qt Widgets**, and **MySQL**.

## Features
- Role-based authentication (Admin & Customer)
- Registration with mandatory email and automatic bank account creation
- SHA-256 password hashing & OTP password recovery via Gmail SMTP
- Fund transfers, withdrawals, AI Credit Scoring, and loan management
- PDF & CSV account statement export with email delivery

## Tech Stack
- C++17, Qt Widgets, MySQL (`mysqlcppconn`), OpenSSL, libcurl, CMake

## Build & Run
1. Create a `.env` file in the project root with the following variables:
   - `DB_HOST` (e.g., `127.0.0.1`)
   - `DB_USER` (MySQL username)
   - `DB_PASSSWORD` (MySQL password)
   - `DB_NAME` (MySQL database name)
   - `SMTP_EMAIL` (Your Gmail address for sending OTP codes & statements)
   - `SMTP_PASSWORD` (16-character **Google App Password**, generated via Google Account → Security → 2-Step Verification → App passwords)
2. Build the project:
   `mkdir -p build && cd build && cmake .. && make`
3. Run the application:
   `export $(cat ../.env | xargs) && ./BankManagmentSystem`