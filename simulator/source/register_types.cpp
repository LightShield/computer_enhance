#include <iomanip>
#include <sstream>
#include <stdexcept>
#include "register_types.h"

Register16::Register16() : value(0) {}

uint8_t& Register16::get8(const std::string& name) {
    if (name.back() == 'H') {
        return high;
    }
    if (name.back() == 'L') {
        return low;
    }
    throw std::runtime_error("Invalid 8-bit register name: " + name);
}

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
