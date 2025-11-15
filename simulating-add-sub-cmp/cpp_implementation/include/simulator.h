// simulator.h
#pragma once
#include <string>
#include "registers.h"

class Simulator {
    Registers regs;

public:
    Simulator();
    void run_simulation(const std::string& filepath);
    std::string run_command(const std::string& line);
    const Registers& get_registers() const { return regs; }
};
