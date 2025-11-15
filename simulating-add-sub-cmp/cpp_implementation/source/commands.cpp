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

// MOV instruction: mov dest, src
std::string cmd_mov(Registers& regs, const std::vector<std::string>& args) {
    if (args.size() != 2) {
        throw std::runtime_error("mov requires 2 arguments");
    }

    std::string dest = clean_operand(args[0]);
    std::string src = clean_operand(args[1]);

    int src_value = parse_operand(regs, src);

    if (regs.is8(dest)) {
        regs.get8(dest) = static_cast<uint8_t>(src_value);
        LOGGER.Debug("mov {}, {} -> {}[8] = {}", dest, src, dest, static_cast<int>(regs.get8(dest)));
    } else if (regs.is16(dest)) {
        regs.get16(dest) = static_cast<uint16_t>(src_value);
        LOGGER.Debug("mov {}, {} -> {}[16] = {}", dest, src, dest, regs.get16(dest));
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
        LOGGER.Debug("add {}, {} -> {}[8] = {} (was {})", dest, src, dest,
                    static_cast<int>(regs.get8(dest)), static_cast<int>(old_val));
    } else if (regs.is16(dest)) {
        uint16_t old_val = regs.get16(dest);
        regs.get16(dest) += static_cast<uint16_t>(src_value);
        LOGGER.Debug("add {}, {} -> {}[16] = {} (was {})", dest, src, dest,
                    regs.get16(dest), old_val);
    } else {
        throw std::runtime_error("Unknown destination register: " + dest);
    }

    // TODO: Update flags (CF, PF, AF, ZF, SF, OF)

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
        LOGGER.Debug("sub {}, {} -> {}[8] = {} (was {})", dest, src, dest,
                    static_cast<int>(regs.get8(dest)), static_cast<int>(old_val));
    } else if (regs.is16(dest)) {
        uint16_t old_val = regs.get16(dest);
        regs.get16(dest) -= static_cast<uint16_t>(src_value);
        LOGGER.Debug("sub {}, {} -> {}[16] = {} (was {})", dest, src, dest,
                    regs.get16(dest), old_val);
    } else {
        throw std::runtime_error("Unknown destination register: " + dest);
    }

    // TODO: Update flags (CF, PF, AF, ZF, SF, OF)

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
    int result = dest_value - src_value;

    LOGGER.Debug("cmp {}, {} -> {} - {} = {}", dest, src, dest_value, src_value, result);

    // TODO: Update flags based on result (CF, PF, AF, ZF, SF, OF)
    regs.flags.ZF = (result == 0) ? 1 : 0;
    regs.flags.SF = (result < 0) ? 1 : 0;

    return "OK";
}
