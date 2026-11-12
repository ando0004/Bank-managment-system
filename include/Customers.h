#pragma once
#include <string>

class Customer {
private:
    int id;
    std::string name;
    std::string surname;
    std::string phone;
    std::string address;

public:
    Customer();
    Customer(const std::string& name, const std::string& surname,
             const std::string& phone, const std::string& address);
    Customer(int id, const std::string& name, const std::string& surname,
             const std::string& phone, const std::string& address);

    int getId() const;
    std::string getName() const;
    std::string getSurname() const;
    std::string getPhone() const;
    std::string getAddress() const;

    void setId(int newId);
    void setName(const std::string& newName);
    void setSurname(const std::string& newSurname);
    void setPhone(const std::string& newPhone);
    void setAddress(const std::string& newAddress);

    void display() const;
};
