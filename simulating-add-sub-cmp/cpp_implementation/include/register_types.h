#pragma once
#include <cstdint>
#include <string>

struct Register16 {
    union {
        uint16_t value;
        struct {
            uint8_t low;
            uint8_t high;
        };
    };

    Register16();

    uint8_t& get8(const std::string& name);
};

struct Flags {
    union {
        uint16_t value;
        struct {
            uint16_t CF : 1;  // Carry
            uint16_t    : 1;  // always 1 in 8086 but ignored here
            uint16_t PF : 1;  // Parity
            uint16_t    : 1;
            uint16_t AF : 1;  // Auxiliary carry
            uint16_t    : 1;
            uint16_t ZF : 1;  // Zero
            uint16_t SF : 1;  // Sign
            uint16_t TF : 1;  // Trap
            uint16_t IF : 1;  // Interrupt enable
            uint16_t DF : 1;  // Direction
            uint16_t OF : 1;  // Overflow
            uint16_t    : 4;  // reserved
        };
    };

    Flags();
    void reset();
    std::string dump() const;
};
