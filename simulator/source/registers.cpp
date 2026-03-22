#include <iomanip>
#include <sstream>
#include <stdexcept>

#include "registers.h"

namespace lightshield {

Registers::Registers() : m_captured_flags_value(0) {
    m_reg16_map = {
        {"ax", &m_ax}, {"bx", &m_bx}, {"cx", &m_cx}, {"dx", &m_dx},
        {"si", &m_si}, {"di", &m_di}, {"bp", &m_bp}, {"sp", &m_sp}, {"ip", &m_ip}
    };

    m_reg8_map = {
        {"ah", &m_ax.high}, {"al", &m_ax.low},
        {"bh", &m_bx.high}, {"bl", &m_bx.low},
        {"ch", &m_cx.high}, {"cl", &m_cx.low},
        {"dh", &m_dx.high}, {"dl", &m_dx.low},
    };
}

Register16Proxy Registers::get16(const std::string& name) {
    auto it = m_reg16_map.find(name);
    if (it == m_reg16_map.end()) {
        throw std::runtime_error("Unknown 16-bit register: " + name);
    }
    return Register16Proxy(*this, name, &(it->second->value));
}

Register8Proxy Registers::get8(const std::string& name) {
    auto it = m_reg8_map.find(name);
    if (it == m_reg8_map.end()) {
        throw std::runtime_error("Unknown 8-bit register: " + name);
    }
    return Register8Proxy(*this, name, it->second);
}

bool Registers::is8(const std::string& name) const {
    return m_reg8_map.count(name) > 0;
}

bool Registers::is16(const std::string& name) const {
    return m_reg16_map.count(name) > 0;
}

std::string Registers::dump() const {
    std::ostringstream out;
    out << std::hex << std::uppercase << std::setfill('0');

    out << "AX=" << std::setw(4) << m_ax.value << " "
        << "BX=" << std::setw(4) << m_bx.value << " "
        << "CX=" << std::setw(4) << m_cx.value << " "
        << "DX=" << std::setw(4) << m_dx.value << " "
        << "SI=" << std::setw(4) << m_si.value << " "
        << "DI=" << std::setw(4) << m_di.value << " "
        << "BP=" << std::setw(4) << m_bp.value << " "
        << "SP=" << std::setw(4) << m_sp.value << " "
        << "IP=" << std::setw(4) << m_ip.value << " | "
        << m_flags.dump();

    return out.str();
}

void Registers::mark_register_change(const std::string& name, uint16_t old_value, uint16_t new_value) {
    if (old_value != new_value) {
        m_change_set.register_changes.push_back({name, old_value, new_value});
    }
}

void Registers::mark_flag_change(const std::string& flag_name, bool old_value, bool new_value) {
    if (old_value != new_value) {
        m_change_set.flags_changes.push_back({flag_name, old_value, new_value});
    }
}

ChangeSet Registers::get_last_changes() {
    ChangeSet result = m_change_set;
    m_change_set.clear();
    return result;
}

void Registers::capture_flags() {
    m_captured_flags_value = m_flags.value;
}

void Registers::check_flag_changes() {
    uint16_t current_flags = m_flags.value;

    const struct {
        const char* name;
        uint16_t mask;
    } flag_bits[] = {
        {"CF", 0x0001},
        {"PF", 0x0004},
        {"AF", 0x0010},
        {"ZF", 0x0040},
        {"SF", 0x0080},
        {"TF", 0x0100},
        {"IF", 0x0200},
        {"DF", 0x0400},
        {"OF", 0x0800}
    };

    for (const auto& flag : flag_bits) {
        bool old_val = (m_captured_flags_value & flag.mask) != 0;
        bool new_val = (current_flags & flag.mask) != 0;
        if (old_val != new_val) {
            mark_flag_change(flag.name, old_val, new_val);
        }
    }
}

} // namespace lightshield
