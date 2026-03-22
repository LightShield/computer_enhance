#include <iomanip>
#include <sstream>
#include <stdexcept>
#include "register_types.h"

namespace lightshield {

Register16::Register16() : value(0) {}

uint8_t& Register16::get8(const std::string& name) {
    if (name.size() != 2) throw std::runtime_error("Invalid 8-bit register name: " + name);
    if (name[1] == 'l') return low;
    if (name[1] == 'h') return high;
    throw std::runtime_error("Invalid 8-bit register name: " + name);
}

Flags::Flags() : value(0) {}

void Flags::reset() { value = 0; }

std::string Flags::dump() const {
    std::string result;
    if (CF) result += "C";
    if (PF) result += "P";
    if (AF) result += "A";
    if (ZF) result += "Z";
    if (SF) result += "S";
    if (TF) result += "T";
    if (IF) result += "I";
    if (DF) result += "D";
    if (OF) result += "O";
    return result;
}

} // namespace lightshield
