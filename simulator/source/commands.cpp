#include <sstream>
#include <stdexcept>
#include <vector>

#include "commands.h"
#include "logger.h"

namespace lightshield {

static std::string clean_operand(const std::string& operand) {
    std::string cleaned = operand;
    if (!cleaned.empty() && cleaned.back() == ',') {
        cleaned.pop_back();
    }
    return cleaned;
}

static bool is_immediate_value(const std::string& operand) {
    if (operand.empty()) return false;
    return std::isdigit(operand[0]) || operand[0] == '-' || (operand.size() > 2 && operand.substr(0, 2) == "0x");
}

static int parse_operand(Registers& regs, const std::string& operand, uint16_t current_instr_end_ip) {
    std::string cleaned = clean_operand(operand);

    if (cleaned.empty()) throw std::runtime_error("Empty operand");

    if (cleaned[0] == '$') {
        int offset = 0;
        if (cleaned.size() > 1) {
            offset = std::stoi(cleaned.substr(1));
        }
        return static_cast<int>(current_instr_end_ip + offset);
    }

    if (is_immediate_value(cleaned)) {
        return std::stoi(cleaned, nullptr, 0);
    }

    if (regs.is8(cleaned)) {
        return regs.get8(cleaned);
    } else if (regs.is16(cleaned)) {
        return regs.get16(cleaned);
    }

    throw std::runtime_error("Unknown operand: " + cleaned);
}

static constexpr uint8_t BITS_IN_BYTE = 8;

static bool calculate_parity(uint8_t value) {
    uint8_t count = 0;
    for (uint8_t i = 0; i < BITS_IN_BYTE; i++) {
        if (value & (1 << i)) count++;
    }
    return (count % 2) == 0;
}

static bool has_signed_overflow_sub(int16_t old_val, int16_t operand, int16_t result) {
    return (old_val >= 0 && operand < 0 && result < 0) ||
           (old_val < 0 && operand >= 0 && result >= 0);
}

static bool has_signed_overflow_add(int16_t old_val, int16_t operand, int16_t result) {
    return (old_val >= 0 && operand >= 0 && result < 0) ||
           (old_val < 0 && operand < 0 && result >= 0);
}

static void update_flags_arithmetic(Registers& regs, uint16_t result, uint16_t old_val, uint16_t operand, bool is_8bit, bool is_sub) {
    regs.m_flags.bits.ZF = (result == 0);

    if (is_8bit) {
        regs.m_flags.bits.SF = ((result & 0x80) != 0);
        regs.m_flags.bits.PF = calculate_parity(static_cast<uint8_t>(result));
    } else {
        regs.m_flags.bits.SF = ((result & 0x8000) != 0);
        regs.m_flags.bits.PF = calculate_parity(static_cast<uint8_t>(result & 0xFF));
    }

    // AF is carry from bit 3 to 4
    if (is_sub) {
        regs.m_flags.bits.AF = ((old_val & 0xF) < (operand & 0xF));
        regs.m_flags.bits.CF = (old_val < operand);
        if (is_8bit) {
            regs.m_flags.bits.OF = has_signed_overflow_sub(
                static_cast<int8_t>(old_val),
                static_cast<int8_t>(operand),
                static_cast<int8_t>(result));
        } else {
            regs.m_flags.bits.OF = has_signed_overflow_sub(
                static_cast<int16_t>(old_val),
                static_cast<int16_t>(operand),
                static_cast<int16_t>(result));
        }
    } else {
        regs.m_flags.bits.AF = (((old_val & 0xF) + (operand & 0xF)) > 0xF);
        if (is_8bit) {
            regs.m_flags.bits.CF = (result < (old_val & 0xFF));
            regs.m_flags.bits.OF = has_signed_overflow_add(
                static_cast<int8_t>(old_val),
                static_cast<int8_t>(operand),
                static_cast<int8_t>(result));
        } else {
            regs.m_flags.bits.CF = (result < old_val);
            regs.m_flags.bits.OF = has_signed_overflow_add(
                static_cast<int16_t>(old_val),
                static_cast<int16_t>(operand),
                static_cast<int16_t>(result));
        }
    }
}

std::string cmd_mov(Registers& regs, const std::vector<std::string>& args, uint16_t current_instr_end_ip) {
    if (args.size() != 2) throw std::runtime_error("mov requires 2 arguments");
    std::string dest = clean_operand(args[0]);
    int src_value = parse_operand(regs, args[1], current_instr_end_ip);

    if (regs.is8(dest)) {
        regs.get8(dest) = static_cast<uint8_t>(src_value);
    } else if (regs.is16(dest)) {
        regs.get16(dest) = static_cast<uint16_t>(src_value);
    } else {
        throw std::runtime_error("Unknown destination register: " + dest);
    }
    return "OK";
}

std::string cmd_add(Registers& regs, const std::vector<std::string>& args, uint16_t current_instr_end_ip) {
    if (args.size() != 2) throw std::runtime_error("add requires 2 arguments");
    std::string dest = clean_operand(args[0]);
    int src_value = parse_operand(regs, args[1], current_instr_end_ip);

    if (regs.is8(dest)) {
        uint8_t old_val = regs.get8(dest);
        regs.get8(dest) = static_cast<uint8_t>(old_val + static_cast<uint8_t>(src_value));
        update_flags_arithmetic(regs, regs.get8(dest), old_val, static_cast<uint8_t>(src_value), true, false);
    } else if (regs.is16(dest)) {
        uint16_t old_val = regs.get16(dest);
        regs.get16(dest) = static_cast<uint16_t>(old_val + static_cast<uint16_t>(src_value));
        update_flags_arithmetic(regs, regs.get16(dest), old_val, static_cast<uint16_t>(src_value), false, false);
    } else {
        throw std::runtime_error("Unknown destination register: " + dest);
    }
    return "OK";
}

std::string cmd_sub(Registers& regs, const std::vector<std::string>& args, uint16_t current_instr_end_ip) {
    if (args.size() != 2) throw std::runtime_error("sub requires 2 arguments");
    std::string dest = clean_operand(args[0]);
    int src_value = parse_operand(regs, args[1], current_instr_end_ip);

    if (regs.is8(dest)) {
        uint8_t old_val = regs.get8(dest);
        regs.get8(dest) = static_cast<uint8_t>(old_val - static_cast<uint8_t>(src_value));
        update_flags_arithmetic(regs, regs.get8(dest), old_val, static_cast<uint8_t>(src_value), true, true);
    } else if (regs.is16(dest)) {
        uint16_t old_val = regs.get16(dest);
        regs.get16(dest) = static_cast<uint16_t>(old_val - static_cast<uint16_t>(src_value));
        update_flags_arithmetic(regs, regs.get16(dest), old_val, static_cast<uint16_t>(src_value), false, true);
    } else {
        throw std::runtime_error("Unknown destination register: " + dest);
    }
    return "OK";
}

std::string cmd_cmp(Registers& regs, const std::vector<std::string>& args, uint16_t current_instr_end_ip) {
    if (args.size() != 2) throw std::runtime_error("cmp requires 2 arguments");
    std::string dest_name = clean_operand(args[0]);
    int dest_value = parse_operand(regs, dest_name, current_instr_end_ip);
    int src_value = parse_operand(regs, args[1], current_instr_end_ip);

    bool is_8bit = regs.is8(dest_name);
    uint16_t result = is_8bit ? static_cast<uint16_t>(static_cast<uint8_t>(dest_value - src_value)) : static_cast<uint16_t>(dest_value - src_value);
    update_flags_arithmetic(regs, result, static_cast<uint16_t>(dest_value), static_cast<uint16_t>(src_value), is_8bit, true);
    return "OK";
}

std::string cmd_jne(Registers& regs, const std::vector<std::string>& args, uint16_t current_instr_end_ip) {
    if (args.size() != 1) throw std::runtime_error("jne requires 1 argument");
    if (!regs.m_flags.bits.ZF) {
        int target_ip = parse_operand(regs, args[0], current_instr_end_ip);
        regs.set_ip(static_cast<uint16_t>(target_ip));
    }
    return "OK";
}

} // namespace lightshield
