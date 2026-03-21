#include "register_proxy.h"
#include "registers.h"

Register16Proxy::Register16Proxy(Registers& r, const std::string& n, uint16_t* p)
    : name(n), regs(r), ptr(p) {}

Register16Proxy& Register16Proxy::operator=(uint16_t value) {
    uint16_t old_value = *ptr;
    *ptr = value;
    regs.mark_register_change(name, old_value, value);
    return *this;
}

Register16Proxy& Register16Proxy::operator+=(uint16_t value) {
    uint16_t old_value = *ptr;
    *ptr += value;
    regs.mark_register_change(name, old_value, *ptr);
    return *this;
}

Register16Proxy& Register16Proxy::operator-=(uint16_t value) {
    uint16_t old_value = *ptr;
    *ptr -= value;
    regs.mark_register_change(name, old_value, *ptr);
    return *this;
}

Register8Proxy::Register8Proxy(Registers& r, const std::string& n, uint8_t* p)
    : name(n), regs(r), ptr(p) {}

Register8Proxy& Register8Proxy::operator=(uint8_t value) {
    uint8_t old_value = *ptr;
    *ptr = value;
    regs.mark_register_change(name, old_value, value);
    return *this;
}

Register8Proxy& Register8Proxy::operator+=(uint8_t value) {
    uint8_t old_value = *ptr;
    *ptr += value;
    regs.mark_register_change(name, old_value, *ptr);
    return *this;
}

Register8Proxy& Register8Proxy::operator-=(uint8_t value) {
    uint8_t old_value = *ptr;
    *ptr -= value;
    regs.mark_register_change(name, old_value, *ptr);
    return *this;
}
