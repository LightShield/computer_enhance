#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include "commands.h"
#include "logger.h"
#include "simulator.h"

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
    int line_num = 0;
    std::vector<std::string> final_section;
    bool in_final_section = false;

    while (std::getline(file, line)) {
        line_num++;

        if (line.find("Final") == 0) {
            LOGGER.Debug("Found 'Final' marker at line {}", line_num);
            in_final_section = true;
            final_section.push_back(line);
            continue;
        }

        if (in_final_section) {
            final_section.push_back(line);
            continue;
        }

        if (line.empty() || line[0] == '-' || std::isspace(line[0])) continue;

        LOGGER.Debug("Processing line {}: {}", line_num, line);

        try {
            CommandLine cmd_line = parse_command_line(line);

            m_regs.capture_flags();
            std::string result = run_command(cmd_line.command);
            m_regs.check_flag_changes();

            ChangeSet changes = m_regs.get_last_changes();
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

            size_t comment_pos = line.find(';');
            std::string display_line = (comment_pos != std::string::npos) ? line.substr(0, comment_pos) : line;
            while (!display_line.empty() && std::isspace(display_line.back())) {
                display_line.pop_back();
            }

            if (change_str.str().empty()) {
                LOGGER.Info("{}", display_line);
            } else {
                LOGGER.Info("{} ; {}", display_line, change_str.str());
            }

            if (cmd_line.has_expected) {
                compare_with_expected(cmd_line.expected);
            }
        } catch (const std::exception& e) {
            LOGGER.Error("Error processing line {}: {}", line_num, e.what());
        }
    }

    LOGGER.Info("");
    if (!final_section.empty()) {
        LOGGER.Info("Final state comparison:");
        compare_final_state(final_section);
    }
}

std::string Simulator::run_command(const std::string& line) {
    auto tokens = split(line);

    if (tokens.empty()) {
        LOGGER.Warn("Empty command line received");
        throw std::runtime_error("Empty command");
    }

    const std::string& cmd = tokens[0];
    uint32_t cmd_hash = hash_command(cmd.c_str());

    LOGGER.Debug("Looking up command: {} (hash: {})", cmd, cmd_hash);

    for (size_t i = 0; i < COMMANDS_TABLE_SIZE; ++i) {
        if (commands_table[i].hash == cmd_hash) {
            // Pass Registers and arguments excluding command itself
            std::vector<std::string> args(tokens.begin() + 1, tokens.end());
            LOGGER.Debug("Executing command '{}' with {} arguments", cmd, args.size());
            return commands_table[i].handler(m_regs, args);
        }
    }

    LOGGER.Error("Unknown command: {}", cmd);
    throw std::runtime_error("Unknown command: " + cmd);
}

CommandLine Simulator::parse_command_line(const std::string& line) {
    CommandLine result;
    result.has_expected = false;

    // Find semicolon separator
    size_t semicolon_pos = line.find(';');
    if (semicolon_pos == std::string::npos) {
        // No expected output, just command
        result.command = line;
        return result;
    }

    // Extract command (before semicolon)
    result.command = line.substr(0, semicolon_pos);

    // Extract expected changes (after semicolon)
    std::string expected_str = line.substr(semicolon_pos + 1);
    result.has_expected = true;

    // Parse expected changes
    // Format: "reg:0xOLD->0xNEW" or "flags:OLD->NEW"
    std::istringstream iss(expected_str);
    std::string token;

    while (iss >> token) {
        size_t colon_pos = token.find(':');
        if (colon_pos == std::string::npos) continue;

        std::string name = token.substr(0, colon_pos);
        std::string change = token.substr(colon_pos + 1);

        if (name == "flags") {
            // Parse flag changes: "->S" (set) or "S->" (clear) or "S->Z" (both)
            size_t arrow_pos = change.find("->");
            if (arrow_pos != std::string::npos) {
                std::string old_flags = change.substr(0, arrow_pos);
                std::string new_flags = change.substr(arrow_pos + 2);

                // Flags cleared (in old but not in new)
                for (char flag : old_flags) {
                    if (new_flags.find(flag) == std::string::npos) {
                        result.expected.flags_cleared.insert(std::string(1, flag));
                    }
                }

                // Flags set (in new but not in old)
                for (char flag : new_flags) {
                    if (old_flags.find(flag) == std::string::npos) {
                        result.expected.flags_set.insert(std::string(1, flag));
                    }
                }
            }
        } else {
            // Parse register change: "0xOLD->0xNEW"
            size_t arrow_pos = change.find("->");
            if (arrow_pos != std::string::npos) {
                std::string new_val_str = change.substr(arrow_pos + 2);
                // Remove "0x" prefix if present
                if (new_val_str.substr(0, 2) == "0x") {
                    new_val_str = new_val_str.substr(2);
                }
                // Parse hex value
                uint16_t new_val = static_cast<uint16_t>(std::stoul(new_val_str, nullptr, 16));
                result.expected.register_changes[name] = new_val;
            }
        }
    }

    return result;
}

static std::string trim(const std::string& str) {
    size_t start = 0;
    while (start < str.size() && std::isspace(str[start])) start++;
    size_t end = str.size();
    while (end > start && std::isspace(str[end - 1])) end--;
    return str.substr(start, end - start);
}

void Simulator::compare_with_expected(const ExpectedState& expected) {
    bool all_match = true;

    for (const auto& [reg_name, expected_value] : expected.register_changes) {
        uint16_t actual_value;
        if (m_regs.is8(reg_name)) {
            actual_value = m_regs.get8(reg_name);
        } else if (m_regs.is16(reg_name)) {
            actual_value = m_regs.get16(reg_name);
        } else {
            LOGGER.Error("Unknown register in expected output: {}", reg_name);
            all_match = false;
            continue;
        }

        if (actual_value != expected_value) {
            LOGGER.Error("MISMATCH: {} expected 0x{:x}, got 0x{:x}",
                        reg_name, expected_value, actual_value);
            all_match = false;
        }
    }

    for (const auto& flag_name : expected.flags_set) {
        bool flag_value = false;
        if (flag_name == "C") flag_value = m_regs.flags.CF;
        else if (flag_name == "P") flag_value = m_regs.flags.PF;
        else if (flag_name == "A") flag_value = m_regs.flags.AF;
        else if (flag_name == "Z") flag_value = m_regs.flags.ZF;
        else if (flag_name == "S") flag_value = m_regs.flags.SF;
        else if (flag_name == "O") flag_value = m_regs.flags.OF;
        else if (flag_name == "D") flag_value = m_regs.flags.DF;
        else if (flag_name == "I") flag_value = m_regs.flags.IF;
        else {
            LOGGER.Error("Unknown flag in expected output: {}", flag_name);
            all_match = false;
            continue;
        }

        if (!flag_value) {
            LOGGER.Error("MISMATCH: Flag {} expected to be set but is clear", flag_name);
            all_match = false;
        }
    }

    for (const auto& flag_name : expected.flags_cleared) {
        bool flag_value = false;
        if (flag_name == "C") flag_value = m_regs.flags.CF;
        else if (flag_name == "P") flag_value = m_regs.flags.PF;
        else if (flag_name == "A") flag_value = m_regs.flags.AF;
        else if (flag_name == "Z") flag_value = m_regs.flags.ZF;
        else if (flag_name == "S") flag_value = m_regs.flags.SF;
        else if (flag_name == "O") flag_value = m_regs.flags.OF;
        else if (flag_name == "D") flag_value = m_regs.flags.DF;
        else if (flag_name == "I") flag_value = m_regs.flags.IF;
        else {
            LOGGER.Error("Unknown flag in expected output: {}", flag_name);
            all_match = false;
            continue;
        }

        if (flag_value) {
            LOGGER.Error("MISMATCH: Flag {} expected to be clear but is set", flag_name);
            all_match = false;
        }
    }

    if (all_match && (expected.register_changes.size() > 0 || expected.flags_set.size() > 0 || expected.flags_cleared.size() > 0)) {
        LOGGER.Debug("All expected changes match!");
    }
}

void Simulator::compare_final_state(const std::vector<std::string>& final_section) {
    std::unordered_map<std::string, uint16_t> expected_regs;
    std::string expected_flags;

    for (const auto& line : final_section) {
        LOGGER.Info("{}", line);

        std::string trimmed = trim(line);
        if (trimmed.find("Final") == 0 || trimmed.empty()) continue;

        size_t colon_pos = trimmed.find(':');
        if (colon_pos == std::string::npos) continue;

        std::string key = trim(trimmed.substr(0, colon_pos));
        std::string value_str = trim(trimmed.substr(colon_pos + 1));

        if (key == "flags") {
            expected_flags = value_str;
        } else {
            size_t hex_pos = value_str.find("0x");
            if (hex_pos != std::string::npos) {
                size_t end_pos = value_str.find(' ', hex_pos);
                std::string hex_val = value_str.substr(hex_pos + 2, end_pos - hex_pos - 2);
                uint16_t reg_value = static_cast<uint16_t>(std::stoul(hex_val, nullptr, 16));
                expected_regs[key] = reg_value;
            }
        }
    }

    LOGGER.Info("");
    LOGGER.Info("Actual final state:");

    bool has_diff = false;
    std::ostringstream actual_output;

    for (const auto& [reg_name, expected_value] : expected_regs) {
        uint16_t actual_value = 0;
        if (m_regs.is16(reg_name)) {
            actual_value = m_regs.get16(reg_name);
        }

        actual_output << "      " << reg_name << ": 0x" << std::hex << std::setw(4) << std::setfill('0')
                     << actual_value << " (" << std::dec << actual_value << ")";

        if (actual_value != expected_value) {
            actual_output << " <-- MISMATCH (expected 0x" << std::hex << expected_value << ")";
            has_diff = true;
        }
        actual_output << "\n";
    }

    std::string actual_flags_str;
    if (m_regs.flags.CF) actual_flags_str += "C";
    if (m_regs.flags.PF) actual_flags_str += "P";
    if (m_regs.flags.AF) actual_flags_str += "A";
    if (m_regs.flags.ZF) actual_flags_str += "Z";
    if (m_regs.flags.SF) actual_flags_str += "S";
    if (m_regs.flags.OF) actual_flags_str += "O";
    if (m_regs.flags.DF) actual_flags_str += "D";
    if (m_regs.flags.IF) actual_flags_str += "I";

    actual_output << "   flags: " << actual_flags_str;
    if (actual_flags_str != expected_flags) {
        actual_output << " <-- MISMATCH (expected " << expected_flags << ")";
        has_diff = true;
    }

    LOGGER.Info("{}", actual_output.str());

    if (!has_diff) {
        LOGGER.Info("\nAll final state values match!");
    }
}
