#include <iomanip>
#include <sstream>
#include <stdexcept>
#include "registers.h"

// ========================
// Register16
// ========================
Register16::Register16() : value(0) {}

uint8_t& Register16::get8(const std::string& name) {
    if (name.back() == 'H') return high;
    if (name.back() == 'L') return low;
    throw std::runtime_error("Invalid 8-bit register name: " + name);
}

// ========================
// Flags
// ========================
Flags::Flags() : value(0) {}

void Flags::reset() {
    value = 0;
}

std::string Flags::dump() const {
    std::ostringstream out;
    out << "FLAGS: "
        << "CF=" << CF << " "
        << "PF=" << PF << " "
        << "AF=" << AF << " "
        << "ZF=" << ZF << " "
        << "SF=" << SF << " "
        << "OF=" << OF << " "
        << "DF=" << DF << " "
        << "IF=" << IF;
    return out.str();
}

// ========================
// Registers
// ========================
Registers::Registers() {
    // Map 16-bit registers
    reg16_map = {
        {"ax", &ax}, {"bx", &bx}, {"cx", &cx}, {"dx", &dx},
        {"si", &si}, {"di", &di}, {"bp", &bp}, {"sp", &sp},
    };

    // Map 8-bit registers
    reg8_map = {
        {"ah", &ax.high}, {"al", &ax.low},
        {"bh", &bx.high}, {"bl", &bx.low},
        {"ch", &cx.high}, {"cl", &cx.low},
        {"dh", &dx.high}, {"dl", &dx.low},
    };
}

Register16Proxy Registers::get16(const std::string& name) {
    auto it = reg16_map.find(name);
    if (it == reg16_map.end())
        throw std::runtime_error("Unknown 16-bit register: " + name);
    return Register16Proxy(*this, name, &(it->second->value));
}

Register8Proxy Registers::get8(const std::string& name) {
    auto it = reg8_map.find(name);
    if (it == reg8_map.end())
        throw std::runtime_error("Unknown 8-bit register: " + name);
    return Register8Proxy(*this, name, it->second);
}

bool Registers::is8(const std::string& name) const {
    return reg8_map.count(name) > 0;
}

bool Registers::is16(const std::string& name) const {
    return reg16_map.count(name) > 0;
}

std::string Registers::dump() const {
    std::ostringstream out;
    out << std::hex << std::uppercase << std::setfill('0');

    out << "AX=" << std::setw(4) << ax.value << " "
        << "BX=" << std::setw(4) << bx.value << " "
        << "CX=" << std::setw(4) << cx.value << " "
        << "DX=" << std::setw(4) << dx.value << " "
        << "SI=" << std::setw(4) << si.value << " "
        << "DI=" << std::setw(4) << di.value << " "
        << "BP=" << std::setw(4) << bp.value << " "
        << "SP=" << std::setw(4) << sp.value << " | "
        << flags.dump();

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
    m_captured_flags_value = flags.value;
}

void Registers::check_flag_changes() {
    uint16_t current_flags = flags.value;

    // Check each flag bit and record changes
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

// ========================
// Register16Proxy operators
// ========================
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

// ========================
// Register8Proxy operators
// ========================
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
