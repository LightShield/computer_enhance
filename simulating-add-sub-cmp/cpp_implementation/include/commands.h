#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include "registers.h"

// Command handler type: takes registers and arguments, returns result string
using CommandHandler = std::string(*)(Registers& regs, const std::vector<std::string>& args);

// Command table entry
struct CommandEntry {
    uint32_t hash;
    CommandHandler handler;
};

// Simple hash function for command names
inline uint32_t hash_command(const char* str) {
    uint32_t hash = 5381;
    while (*str) {
        hash = ((hash << 5) + hash) + static_cast<unsigned char>(*str);
        str++;
    }
    return hash;
}

// Forward declarations of command handlers
std::string cmd_mov(Registers& regs, const std::vector<std::string>& args);
std::string cmd_add(Registers& regs, const std::vector<std::string>& args);
std::string cmd_sub(Registers& regs, const std::vector<std::string>& args);
std::string cmd_cmp(Registers& regs, const std::vector<std::string>& args);

// Command table
constexpr size_t commands_table_size = 4;
inline CommandEntry commands_table[commands_table_size] = {
    {hash_command("mov"), cmd_mov},
    {hash_command("add"), cmd_add},
    {hash_command("sub"), cmd_sub},
    {hash_command("cmp"), cmd_cmp},
};
