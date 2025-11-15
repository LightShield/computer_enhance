#pragma once
#include <cstdint>
#include <string>

struct Registers;

struct Register16Proxy {
    std::string name;
    Registers& regs;
    uint16_t* ptr;

    Register16Proxy(Registers& r, const std::string& n, uint16_t* p);

    Register16Proxy& operator=(uint16_t value);
    Register16Proxy& operator+=(uint16_t value);
    Register16Proxy& operator-=(uint16_t value);

    operator uint16_t() const { return *ptr; }
};

struct Register8Proxy {
    std::string name;
    Registers& regs;
    uint8_t* ptr;

    Register8Proxy(Registers& r, const std::string& n, uint8_t* p);

    Register8Proxy& operator=(uint8_t value);
    Register8Proxy& operator+=(uint8_t value);
    Register8Proxy& operator-=(uint8_t value);

    operator uint8_t() const { return *ptr; }
};
