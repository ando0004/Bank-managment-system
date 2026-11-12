#include "Customers.h"
#include <iostream>
Customer::Customer()
    : id(0), name(""), surname(""), phone(""), address("") {}

Customer::Customer(const std::string& name, const std::string& surname,
                   const std::string& phone, const std::string& address)
    : id(0), name(name), surname(surname), phone(phone), address(address) {}

Customer::Customer(int id, const std::string& name, const std::string& surname,
                   const std::string& phone, const std::string& address)
    : id(id), name(name), surname(surname), phone(phone), address(address) {}

int Customer::getId() const{return id;}
std::string Customer::getName() const{return name;}
std::string Customer::getSurname() const{return surname;}
std::string Customer::getPhone() const{return phone;}
std::string Customer::getAddress() const{return address;}
void Customer::setId(int newId) {id = newId;}
void Customer::setName(const std::string& newName) {name = newName;}
void Customer::setSurname(const std::string& newSurname) {surname = newSurname;}
void Customer::setPhone(const std::string& newPhone) {phone = newPhone;}
void Customer::setAddress(const std::string& newAddress) {address = newAddress;}
void Customer::display() const {
    std::cout << "ID: " << id
              << " | Name: " << name << " " << surname
              << " | Phone: " << phone
              << " | Address: " << address << std::endl;
}
