#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "registers.h"

// Represents expected state changes for a command
struct ExpectedState {
    std::unordered_map<std::string, uint16_t> register_changes;  // reg_name -> new_value
    std::unordered_set<std::string> flags_set;                   // Flags that should be set
    std::unordered_set<std::string> flags_cleared;               // Flags that should be cleared
};

// Represents a parsed command line with expected output
struct CommandLine {
    std::string command;
    ExpectedState expected;
    bool has_expected;
};

class Simulator {
    Registers m_regs;

public:
    Simulator();
    void run_simulation(const std::string& filepath);
    std::string run_command(const std::string& line);
    const Registers& get_registers() const { return m_regs; }

private:
    CommandLine parse_command_line(const std::string& line);
    void compare_with_expected(const ExpectedState& expected);
};
