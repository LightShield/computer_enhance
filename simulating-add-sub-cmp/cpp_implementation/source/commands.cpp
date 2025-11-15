#include <sstream>
#include <stdexcept>
#include "commands.h"
#include "logger.h"

static std::string clean_operand(const std::string& operand) {
    std::string cleaned = operand;
    if (!cleaned.empty() && cleaned.back() == ',') {
        cleaned.pop_back();
    }
    return cleaned;
}

static bool is_immediate_value(const std::string& operand) {
    return std::isdigit(operand[0]) || operand[0] == '-';
}

static int parse_operand(Registers& regs, const std::string& operand) {
    std::string cleaned = clean_operand(operand);

    if (cleaned.empty()) throw std::runtime_error("Empty operand");

    if (is_immediate_value(cleaned)) {
        return std::stoi(cleaned);
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
    regs.flags.ZF = (result == 0) ? 1 : 0;

    if (is_8bit) {
        regs.flags.SF = ((result & 0x80) != 0) ? 1 : 0;
        regs.flags.PF = calculate_parity(static_cast<uint8_t>(result)) ? 1 : 0;
    } else {
        regs.flags.SF = ((result & 0x8000) != 0) ? 1 : 0;
        regs.flags.PF = calculate_parity(static_cast<uint8_t>(result & 0xFF)) ? 1 : 0;
    }

    if (is_sub) {
        regs.flags.CF = (old_val < operand) ? 1 : 0;
        if (is_8bit) {
            regs.flags.OF = has_signed_overflow_sub(
                static_cast<int8_t>(old_val),
                static_cast<int8_t>(operand),
                static_cast<int8_t>(result)) ? 1 : 0;
        } else {
            regs.flags.OF = has_signed_overflow_sub(
                static_cast<int16_t>(old_val),
                static_cast<int16_t>(operand),
                static_cast<int16_t>(result)) ? 1 : 0;
        }
    } else {
        if (is_8bit) {
            regs.flags.CF = (result < (old_val & 0xFF)) ? 1 : 0;
            regs.flags.OF = has_signed_overflow_add(
                static_cast<int8_t>(old_val),
                static_cast<int8_t>(operand),
                static_cast<int8_t>(result)) ? 1 : 0;
        } else {
            regs.flags.CF = (result < old_val) ? 1 : 0;
            regs.flags.OF = has_signed_overflow_add(
                static_cast<int16_t>(old_val),
                static_cast<int16_t>(operand),
                static_cast<int16_t>(result)) ? 1 : 0;
        }
    }
}

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

static bool is_8bit_comparison(const Registers& regs, const std::string& dest, int dest_value) {
    return regs.is8(dest) || (dest_value >= -128 && dest_value <= 255);
}

std::string cmd_cmp(Registers& regs, const std::vector<std::string>& args) {
    if (args.size() != 2) {
        throw std::runtime_error("cmp requires 2 arguments");
    }

    std::string dest = clean_operand(args[0]);
    std::string src = clean_operand(args[1]);

    int dest_value = parse_operand(regs, dest);
    int src_value = parse_operand(regs, src);

    bool is_8bit = is_8bit_comparison(regs, dest, dest_value);

    uint16_t result;
    if (is_8bit) {
        result = static_cast<uint8_t>(dest_value) - static_cast<uint8_t>(src_value);
    } else {
        result = static_cast<uint16_t>(dest_value) - static_cast<uint16_t>(src_value);
    }

    LOGGER.Debug("cmp {}, {} -> {} - {} = {}", dest, src, dest_value, src_value, static_cast<int16_t>(result));

    update_flags_arithmetic(regs, result, static_cast<uint16_t>(dest_value), static_cast<uint16_t>(src_value), is_8bit, true);

    return "OK";
}
