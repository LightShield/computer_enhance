#include "commands.h"
#include "logger.h"
#include <stdexcept>
#include <sstream>

// Helper to clean operand (remove commas, whitespace, etc.)
static std::string clean_operand(const std::string& operand) {
    std::string cleaned = operand;
    // Remove trailing comma if present
    if (!cleaned.empty() && cleaned.back() == ',') {
        cleaned.pop_back();
    }
    return cleaned;
}

// Helper to parse immediate value or register
static int parse_operand(Registers& regs, const std::string& operand) {
    std::string cleaned = clean_operand(operand);

    if (cleaned.empty()) throw std::runtime_error("Empty operand");

    // Check if it's an immediate value (number)
    if (std::isdigit(cleaned[0]) || cleaned[0] == '-') {
        return std::stoi(cleaned);
    }

    // Otherwise it's a register
    if (regs.is8(cleaned)) {
        return regs.get8(cleaned);
    } else if (regs.is16(cleaned)) {
        return regs.get16(cleaned);
    }

    throw std::runtime_error("Unknown operand: " + cleaned);
}

// Helper to update flags after arithmetic operations
static void update_flags_arithmetic(Registers& regs, uint16_t result, uint16_t old_val, uint16_t operand, bool is_8bit, bool is_sub) {
    // Zero flag: result is zero
    regs.flags.ZF = (result == 0) ? 1 : 0;

    // Sign flag: high bit is set
    if (is_8bit) {
        regs.flags.SF = ((result & 0x80) != 0) ? 1 : 0;
        // Parity flag: even number of 1 bits in low byte
        uint8_t byte_val = static_cast<uint8_t>(result);
        int parity = 0;
        for (int i = 0; i < 8; i++) {
            if (byte_val & (1 << i)) parity++;
        }
        regs.flags.PF = (parity % 2 == 0) ? 1 : 0;
    } else {
        regs.flags.SF = ((result & 0x8000) != 0) ? 1 : 0;
        // Parity flag: even number of 1 bits in low byte
        uint8_t low_byte = static_cast<uint8_t>(result & 0xFF);
        int parity = 0;
        for (int i = 0; i < 8; i++) {
            if (low_byte & (1 << i)) parity++;
        }
        regs.flags.PF = (parity % 2 == 0) ? 1 : 0;
    }

    // Carry and Overflow flags depend on operation
    if (is_sub) {
        // For SUB: CF set if borrow occurred (unsigned underflow)
        regs.flags.CF = (old_val < operand) ? 1 : 0;
        // OF set if signed overflow occurred
        if (is_8bit) {
            int8_t signed_old = static_cast<int8_t>(old_val);
            int8_t signed_op = static_cast<int8_t>(operand);
            int8_t signed_result = static_cast<int8_t>(result);
            regs.flags.OF = ((signed_old >= 0 && signed_op < 0 && signed_result < 0) ||
                            (signed_old < 0 && signed_op >= 0 && signed_result >= 0)) ? 1 : 0;
        } else {
            int16_t signed_old = static_cast<int16_t>(old_val);
            int16_t signed_op = static_cast<int16_t>(operand);
            int16_t signed_result = static_cast<int16_t>(result);
            regs.flags.OF = ((signed_old >= 0 && signed_op < 0 && signed_result < 0) ||
                            (signed_old < 0 && signed_op >= 0 && signed_result >= 0)) ? 1 : 0;
        }
    } else {
        // For ADD: CF set if carry occurred (unsigned overflow)
        if (is_8bit) {
            regs.flags.CF = (result < (old_val & 0xFF)) ? 1 : 0;
        } else {
            regs.flags.CF = (result < old_val) ? 1 : 0;
        }
        // OF set if signed overflow occurred
        if (is_8bit) {
            int8_t signed_old = static_cast<int8_t>(old_val);
            int8_t signed_op = static_cast<int8_t>(operand);
            int8_t signed_result = static_cast<int8_t>(result);
            regs.flags.OF = ((signed_old >= 0 && signed_op >= 0 && signed_result < 0) ||
                            (signed_old < 0 && signed_op < 0 && signed_result >= 0)) ? 1 : 0;
        } else {
            int16_t signed_old = static_cast<int16_t>(old_val);
            int16_t signed_op = static_cast<int16_t>(operand);
            int16_t signed_result = static_cast<int16_t>(result);
            regs.flags.OF = ((signed_old >= 0 && signed_op >= 0 && signed_result < 0) ||
                            (signed_old < 0 && signed_op < 0 && signed_result >= 0)) ? 1 : 0;
        }
    }
}

// MOV instruction: mov dest, src
std::string cmd_mov(Registers& regs, const std::vector<std::string>& args) {
    if (args.size() != 2) {
        throw std::runtime_error("mov requires 2 arguments");
    }

    std::string dest = clean_operand(args[0]);
    std::string src = clean_operand(args[1]);

    int src_value = parse_operand(regs, src);

    if (regs.is8(dest)) {
        regs.get8(dest) = static_cast<uint8_t>(src_value);  // Proxy tracks change automatically
        LOGGER.Debug("mov {}, {} -> {}[8] = {}", dest, src, dest, static_cast<int>(static_cast<uint8_t>(regs.get8(dest))));
    } else if (regs.is16(dest)) {
        regs.get16(dest) = static_cast<uint16_t>(src_value);  // Proxy tracks change automatically
        LOGGER.Debug("mov {}, {} -> {}[16] = {}", dest, src, dest, static_cast<uint16_t>(regs.get16(dest)));
    } else {
        throw std::runtime_error("Unknown destination register: " + dest);
    }

    return "OK";
}

// ADD instruction: add dest, src
std::string cmd_add(Registers& regs, const std::vector<std::string>& args) {
    if (args.size() != 2) {
        throw std::runtime_error("add requires 2 arguments");
    }

    std::string dest = clean_operand(args[0]);
    std::string src = clean_operand(args[1]);

    int src_value = parse_operand(regs, src);

    if (regs.is8(dest)) {
        uint8_t old_val = regs.get8(dest);
        regs.get8(dest) += static_cast<uint8_t>(src_value);
        uint8_t new_val = regs.get8(dest);
        update_flags_arithmetic(regs, new_val, old_val, static_cast<uint8_t>(src_value), true, false);
        LOGGER.Debug("add {}, {} -> {}[8] = {} (was {})", dest, src, dest,
                    static_cast<int>(new_val), static_cast<int>(old_val));
    } else if (regs.is16(dest)) {
        uint16_t old_val = regs.get16(dest);
        regs.get16(dest) += static_cast<uint16_t>(src_value);
        uint16_t new_val = regs.get16(dest);
        update_flags_arithmetic(regs, new_val, old_val, static_cast<uint16_t>(src_value), false, false);
        LOGGER.Debug("add {}, {} -> {}[16] = {} (was {})", dest, src, dest, new_val, old_val);
    } else {
        throw std::runtime_error("Unknown destination register: " + dest);
    }

    return "OK";
}

// SUB instruction: sub dest, src
std::string cmd_sub(Registers& regs, const std::vector<std::string>& args) {
    if (args.size() != 2) {
        throw std::runtime_error("sub requires 2 arguments");
    }

    std::string dest = clean_operand(args[0]);
    std::string src = clean_operand(args[1]);

    int src_value = parse_operand(regs, src);

    if (regs.is8(dest)) {
        uint8_t old_val = regs.get8(dest);
        regs.get8(dest) -= static_cast<uint8_t>(src_value);
        uint8_t new_val = regs.get8(dest);
        update_flags_arithmetic(regs, new_val, old_val, static_cast<uint8_t>(src_value), true, true);
        LOGGER.Debug("sub {}, {} -> {}[8] = {} (was {})", dest, src, dest,
                    static_cast<int>(new_val), static_cast<int>(old_val));
    } else if (regs.is16(dest)) {
        uint16_t old_val = regs.get16(dest);
        regs.get16(dest) -= static_cast<uint16_t>(src_value);
        uint16_t new_val = regs.get16(dest);
        update_flags_arithmetic(regs, new_val, old_val, static_cast<uint16_t>(src_value), false, true);
        LOGGER.Debug("sub {}, {} -> {}[16] = {} (was {})", dest, src, dest, new_val, old_val);
    } else {
        throw std::runtime_error("Unknown destination register: " + dest);
    }

    return "OK";
}

// CMP instruction: cmp dest, src (like SUB but doesn't modify dest)
std::string cmd_cmp(Registers& regs, const std::vector<std::string>& args) {
    if (args.size() != 2) {
        throw std::runtime_error("cmp requires 2 arguments");
    }

    std::string dest = clean_operand(args[0]);
    std::string src = clean_operand(args[1]);

    int dest_value = parse_operand(regs, dest);
    int src_value = parse_operand(regs, src);

    // Determine if we're comparing 8-bit or 16-bit values
    bool is_8bit = regs.is8(dest) || (dest_value >= -128 && dest_value <= 255);

    // Compute result as if we did a subtraction
    uint16_t result;
    if (is_8bit) {
        result = static_cast<uint8_t>(dest_value) - static_cast<uint8_t>(src_value);
    } else {
        result = static_cast<uint16_t>(dest_value) - static_cast<uint16_t>(src_value);
    }

    LOGGER.Debug("cmp {}, {} -> {} - {} = {}", dest, src, dest_value, src_value, static_cast<int16_t>(result));

    // Update flags as if we did a subtraction (but don't modify the register)
    update_flags_arithmetic(regs, result, static_cast<uint16_t>(dest_value), static_cast<uint16_t>(src_value), is_8bit, true);

    return "OK";
}
