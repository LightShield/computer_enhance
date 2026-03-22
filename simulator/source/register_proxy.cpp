#include "register_proxy.h"
#include "registers.h"

namespace lightshield {

Register16Proxy& Register16Proxy::operator=(uint16_t new_value) {
    uint16_t old_value = *m_value_ptr;
    *m_value_ptr = new_value;
    m_regs.mark_register_change(m_name, old_value, new_value);
    return *this;
}

Register16Proxy& Register16Proxy::operator+=(uint16_t val) {
    return *this = (*m_value_ptr + val);
}

Register16Proxy& Register16Proxy::operator-=(uint16_t val) {
    return *this = (*m_value_ptr - val);
}

Register8Proxy& Register8Proxy::operator=(uint8_t new_value) {
    uint8_t old_value = *m_value_ptr;
    *m_value_ptr = new_value;
    m_regs.mark_register_change(m_name, old_value, new_value);
    return *this;
}

Register8Proxy& Register8Proxy::operator+=(uint8_t val) {
    return *this = static_cast<uint8_t>(*m_value_ptr + val);
}

Register8Proxy& Register8Proxy::operator-=(uint8_t val) {
    return *this = static_cast<uint8_t>(*m_value_ptr - val);
}

} // namespace lightshield
