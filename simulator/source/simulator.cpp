#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <unordered_map>

#include "commands.h"
#include "logger.h"
#include "simulator.h"

namespace lightshield {

static std::vector<std::string> split(const std::string& s) {
    std::istringstream iss(s);
    std::vector<std::string> tokens;
    std::string token;
    while (iss >> token) tokens.push_back(token);
    return tokens;
}

Simulator::Simulator() : m_regs() {}

void Simulator::run_simulation(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file) {
        LOGGER.Error("Cannot open file: {}", filepath);
        throw std::runtime_error("Cannot open file: " + filepath);
    }

    LOGGER.Info("Starting simulation from file: {}", filepath);

    std::string line;
    std::vector<std::string> final_section;
    bool in_final_section = false;
    
    std::unordered_map<uint16_t, CommandLine> ip_to_cmd;
    uint16_t current_parsing_ip = 0;

    while (std::getline(file, line)) {
        if (line.find("Final") == 0) {
            in_final_section = true;
            final_section.push_back(line);
            continue;
        }

        if (in_final_section) {
            final_section.push_back(line);
            continue;
        }

        if (line.empty() || line[0] == '-' || std::isspace(line[0])) continue;

        CommandLine cmd_line = parse_command_line(line);
        ip_to_cmd[current_parsing_ip] = cmd_line;
        
        // Advance IP based on instruction length for next line in trace parsing
        current_parsing_ip += cmd_line.instruction_length;
    }

    m_regs.set_ip(0);
    while (ip_to_cmd.count(m_regs.get_ip())) {
        uint16_t old_ip = m_regs.get_ip();
        CommandLine& cmd_line = ip_to_cmd[old_ip];
        uint16_t next_ip = old_ip + cmd_line.instruction_length;

        try {
            m_regs.capture_flags();
            
            // Increment IP BEFORE running command so $ points to next instruction
            m_regs.set_ip(next_ip);
            
            run_command(cmd_line.command, next_ip);
            
            uint16_t current_ip = m_regs.get_ip();
            m_regs.check_flag_changes();

            ChangeSet changes = m_regs.get_last_changes();
            // Report IP change manually
            changes.register_changes.push_back({"ip", old_ip, current_ip});

            std::ostringstream change_str;
            if (changes.has_changes()) {
                for (const auto& reg_change : changes.register_changes) {
                    change_str << reg_change.name << ":0x" << std::hex << reg_change.old_value
                              << "->0x" << reg_change.new_value << " ";
                }
                for (const auto& flag_change : changes.flags_changes) {
                    change_str << flag_change.flag_name << ":"
                              << (flag_change.old_value ? "1" : "0") << "->"
                              << (flag_change.new_value ? "1" : "0") << " ";
                }
            }

            size_t comment_pos = cmd_line.command.find(';');
            std::string display_line = (comment_pos != std::string::npos) ? cmd_line.command.substr(0, comment_pos) : cmd_line.command;
            while (!display_line.empty() && std::isspace(display_line.back())) {
                display_line.pop_back();
            }

            if (change_str.str().empty()) {
                LOGGER.Info(display_line);
            } else {
                LOGGER.Info(display_line + " ; " + change_str.str());
            }

            if (cmd_line.has_expected) {
                compare_with_expected(cmd_line.expected);
            }
            
            // Safety break if IP points outside our parsed instructions
            if (!ip_to_cmd.count(m_regs.get_ip()) && m_regs.get_ip() > current_parsing_ip) {
                break;
            }
        } catch (const std::exception& e) {
            LOGGER.Error("Error at IP {}: {}", old_ip, e.what());
            break;
        }
    }

    LOGGER.Info("");
    if (!final_section.empty()) {
        LOGGER.Info("Final state comparison:");
        compare_final_state(final_section);
    }
}

std::string Simulator::run_command(const std::string& line, uint16_t current_instr_end_ip) {
    auto tokens = split(line);

    if (tokens.empty()) {
        throw std::runtime_error("Empty command");
    }

    const std::string& cmd = tokens[0];
    uint32_t cmd_hash = hash_command(cmd.c_str());

    for (size_t i = 0; i < COMMANDS_TABLE_SIZE; ++i) {
        if (commands_table[i].hash == cmd_hash) {
            std::vector<std::string> args(tokens.begin() + 1, tokens.end());
            return commands_table[i].handler(m_regs, args, current_instr_end_ip);
        }
    }

    throw std::runtime_error("Unknown command: " + cmd);
}

CommandLine Simulator::parse_command_line(const std::string& line) {
    CommandLine result;
    result.has_expected = false;
    result.instruction_length = 0;

    size_t semicolon_pos = line.find(';');
    result.command = (semicolon_pos == std::string::npos) ? line : line.substr(0, semicolon_pos);

    if (semicolon_pos != std::string::npos) {
        result.has_expected = true;
        std::string expected_str = line.substr(semicolon_pos + 1);
        std::istringstream iss(expected_str);
        std::string token;

        while (iss >> token) {
            size_t colon_pos = token.find(':');
            if (colon_pos == std::string::npos) continue;

            std::string name = token.substr(0, colon_pos);
            std::string change = token.substr(colon_pos + 1);

            if (name == "flags") {
                size_t arrow_pos = change.find("->");
                if (arrow_pos != std::string::npos) {
                    std::string old_flags = change.substr(0, arrow_pos);
                    std::string new_flags = change.substr(arrow_pos + 2);
                    for (char flag : old_flags) {
                        if (new_flags.find(flag) == std::string::npos) result.expected.flags_cleared.insert(std::string(1, flag));
                    }
                    for (char flag : new_flags) {
                        if (old_flags.find(flag) == std::string::npos) result.expected.flags_set.insert(std::string(1, flag));
                    }
                }
            } else if (name == "ip") {
                size_t arrow_pos = change.find("->");
                if (arrow_pos != std::string::npos) {
                    std::string old_ip_str = change.substr(0, arrow_pos);
                    std::string new_ip_str = change.substr(arrow_pos + 2);
                    if (old_ip_str.substr(0, 2) == "0x") old_ip_str = old_ip_str.substr(2);
                    if (new_ip_str.substr(0, 2) == "0x") new_ip_str = new_ip_str.substr(2);
                    uint16_t old_ip = static_cast<uint16_t>(std::stoul(old_ip_str, nullptr, 16));
                    uint16_t new_ip = static_cast<uint16_t>(std::stoul(new_ip_str, nullptr, 16));
                    result.expected.register_changes["ip"] = new_ip;
                    if (new_ip > old_ip) result.instruction_length = new_ip - old_ip;
                }
            } else {
                size_t arrow_pos = change.find("->");
                if (arrow_pos != std::string::npos) {
                    std::string new_val_str = change.substr(arrow_pos + 2);
                    if (new_val_str.substr(0, 2) == "0x") new_val_str = new_val_str.substr(2);
                    result.expected.register_changes[name] = static_cast<uint16_t>(std::stoul(new_val_str, nullptr, 16));
                }
            }
        }
    }

    return result;
}

static std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

void Simulator::compare_with_expected(const ExpectedState& expected) {
    for (const auto& [reg_name, expected_value] : expected.register_changes) {
        uint16_t actual_value = (reg_name == "ip") ? m_regs.get_ip() : (m_regs.is8(reg_name) ? m_regs.get8(reg_name) : m_regs.get16(reg_name));
        if (actual_value != expected_value) {
            LOGGER.Error("MISMATCH: {} expected 0x{:x}, got 0x{:x}", reg_name, expected_value, actual_value);
        }
    }
    
    auto check_flag = [&](const std::string& name, bool expected_set) {
        bool actual = false;
        if (name == "C") actual = m_regs.m_flags.CF;
        else if (name == "P") actual = m_regs.m_flags.PF;
        else if (name == "A") actual = m_regs.m_flags.AF;
        else if (name == "Z") actual = m_regs.m_flags.ZF;
        else if (name == "S") actual = m_regs.m_flags.SF;
        else if (name == "O") actual = m_regs.m_flags.OF;
        
        if (actual != expected_set) {
            LOGGER.Error("MISMATCH: Flag {} expected to be {} but is {}", name, expected_set ? "set" : "clear", actual ? "set" : "clear");
        }
    };

    for (const auto& f : expected.flags_set) check_flag(f, true);
    for (const auto& f : expected.flags_cleared) check_flag(f, false);
}

void Simulator::compare_final_state(const std::vector<std::string>& final_section) {
    std::unordered_map<std::string, uint16_t> expected_regs;
    std::string expected_flags;

    for (const auto& line : final_section) {
        std::string trimmed = trim(line);
        if (trimmed.find("Final") == 0 || trimmed.empty()) continue;
        size_t colon_pos = trimmed.find(':');
        if (colon_pos == std::string::npos) continue;

        std::string key = trim(trimmed.substr(0, colon_pos));
        std::string val_str = trim(trimmed.substr(colon_pos + 1));

        if (key == "flags") expected_flags = val_str;
        else {
            size_t hex_pos = val_str.find("0x");
            if (hex_pos != std::string::npos) {
                std::string hex_val = val_str.substr(hex_pos + 2, 4);
                expected_regs[key] = static_cast<uint16_t>(std::stoul(hex_val, nullptr, 16));
            }
        }
    }

    bool has_diff = false;
    for (const auto& [name, exp] : expected_regs) {
        uint16_t actual = (name == "ip") ? m_regs.get_ip() : m_regs.get16(name);
        if (actual != exp) {
            LOGGER.Error("Final {} MISMATCH: expected 0x{:x}, got 0x{:x}", name, exp, actual);
            has_diff = true;
        }
    }
    
    std::string actual_flags;
    if (m_regs.m_flags.CF) actual_flags += "C";
    if (m_regs.m_flags.PF) actual_flags += "P";
    if (m_regs.m_flags.AF) actual_flags += "A";
    if (m_regs.m_flags.ZF) actual_flags += "Z";
    if (m_regs.m_flags.SF) actual_flags += "S";
    if (m_regs.m_flags.OF) actual_flags += "O";

    if (actual_flags != expected_flags) {
        LOGGER.Error("Final flags MISMATCH: expected {}, got {}", expected_flags, actual_flags);
        has_diff = true;
    }

    if (!has_diff) LOGGER.Info("All final state values match!");
}

} // namespace lightshield
