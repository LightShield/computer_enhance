#include "registers.h"
#include <sstream>
#include <iomanip>
#include <stdexcept>

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

uint16_t& Registers::get16(const std::string& name) {
    auto it = reg16_map.find(name);
    if (it == reg16_map.end())
        throw std::runtime_error("Unknown 16-bit register: " + name);
    return it->second->value;
}

uint8_t& Registers::get8(const std::string& name) {
    auto it = reg8_map.find(name);
    if (it == reg8_map.end())
        throw std::runtime_error("Unknown 8-bit register: " + name);
    return *it->second;
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
