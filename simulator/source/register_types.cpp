#include <iomanip>
#include <sstream>
#include <stdexcept>
#include "register_types.h"

namespace lightshield {

Register16::Register16() : value(0) {}

uint8_t& Register16::get8(const std::string& name) {
    if (name.size() != 2) throw std::runtime_error("Invalid 8-bit register name: " + name);
    if (name[1] == 'l') return bytes.low;
    if (name[1] == 'h') return bytes.high;
    throw std::runtime_error("Invalid 8-bit register name: " + name);
}

Flags::Flags() : value(0) {}

void Flags::reset() { value = 0; }

std::string Flags::dump() const {
    std::string result;
    if (bits.CF) result += "C";
    if (bits.PF) result += "P";
    if (bits.AF) result += "A";
    if (bits.ZF) result += "Z";
    if (bits.SF) result += "S";
    if (bits.TF) result += "T";
    if (bits.IF) result += "I";
    if (bits.DF) result += "D";
    if (bits.OF) result += "O";
    return result;
}

} // namespace lightshield
