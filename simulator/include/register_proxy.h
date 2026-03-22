#pragma once
#include <cstdint>
#include <string>

namespace lightshield {

class Registers;

class Register16Proxy {
public:
    Register16Proxy(Registers& regs, const std::string& name, uint16_t* value_ptr)
        : m_regs(regs), m_name(name), m_value_ptr(value_ptr) {}

    operator uint16_t() const { return *m_value_ptr; }
    Register16Proxy& operator=(uint16_t new_value);
    Register16Proxy& operator+=(uint16_t val);
    Register16Proxy& operator-=(uint16_t val);

private:
    Registers& m_regs;
    std::string m_name;
    uint16_t* m_value_ptr;
};

class Register8Proxy {
public:
    Register8Proxy(Registers& regs, const std::string& name, uint8_t* value_ptr)
        : m_regs(regs), m_name(name), m_value_ptr(value_ptr) {}

    operator uint8_t() const { return *m_value_ptr; }
    Register8Proxy& operator=(uint8_t new_value);
    Register8Proxy& operator+=(uint8_t val);
    Register8Proxy& operator-=(uint8_t val);

private:
    Registers& m_regs;
    std::string m_name;
    uint8_t* m_value_ptr;
};

} // namespace lightshield
