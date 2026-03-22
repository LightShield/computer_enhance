#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "registers.h"

namespace lightshield {

// Represents expected state changes for a command
struct ExpectedState {
    std::unordered_map<std::string, uint16_t> register_changes;
    std::unordered_set<std::string> flags_set;
    std::unordered_set<std::string> flags_cleared;
};

// Represents a parsed command line with expected output
struct CommandLine {
    ExpectedState expected;
    std::string command;
    uint16_t instruction_length;
    bool has_expected;
    
    // Trailing padding for 8-byte alignment
    uint8_t m_reserved[5] = {0}; 
};

class Simulator {
public:
    Simulator();
    void run_simulation(const std::string& filepath);
    std::string run_command(const std::string& line, uint16_t current_instr_end_ip);
    const Registers& get_registers() const { return m_regs; }

private:
    Registers m_regs;

    CommandLine parse_command_line(const std::string& line);
    void compare_with_expected(const ExpectedState& expected);
    void compare_final_state(const std::vector<std::string>& final_section);
};

} // namespace lightshield
