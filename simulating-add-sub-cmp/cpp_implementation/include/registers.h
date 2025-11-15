#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

// ========================
// 16-bit Register
// ========================
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

// ========================
// Flags Register (bitfield)
// ========================
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

// ========================
// Register change tracking
// ========================
struct RegisterChange {
    std::string name;
    uint16_t old_value;
    uint16_t new_value;
};

struct FlagsChange {
    std::string flag_name;
    bool old_value;
    bool new_value;
};

struct ChangeSet {
    std::vector<RegisterChange> register_changes;
    std::vector<FlagsChange> flags_changes;

    bool has_changes() const {
        return !register_changes.empty() || !flags_changes.empty();
    }

    void clear() {
        register_changes.clear();
        flags_changes.clear();
    }
};

// Forward declaration
struct Registers;

// ========================
// Register proxies for automatic change tracking
// ========================
struct Register16Proxy {
    Registers& regs;
    std::string name;
    uint16_t* ptr;

    Register16Proxy(Registers& r, const std::string& n, uint16_t* p)
        : regs(r), name(n), ptr(p) {}

    // Assignment operator - tracks change automatically
    Register16Proxy& operator=(uint16_t value);

    // Compound assignment operators - track changes automatically
    Register16Proxy& operator+=(uint16_t value);
    Register16Proxy& operator-=(uint16_t value);

    // Implicit conversion for reading
    operator uint16_t() const { return *ptr; }
};

struct Register8Proxy {
    Registers& regs;
    std::string name;
    uint8_t* ptr;

    Register8Proxy(Registers& r, const std::string& n, uint8_t* p)
        : regs(r), name(n), ptr(p) {}

    // Assignment operator - tracks change automatically
    Register8Proxy& operator=(uint8_t value);

    // Compound assignment operators - track changes automatically
    Register8Proxy& operator+=(uint8_t value);
    Register8Proxy& operator-=(uint8_t value);

    // Implicit conversion for reading
    operator uint8_t() const { return *ptr; }
};

// ========================
// Registers block
// ========================
struct Registers {
    Register16 ax, bx, cx, dx, si, di, bp, sp;
    Flags flags;

    std::unordered_map<std::string, Register16*> reg16_map;
    std::unordered_map<std::string, uint8_t*> reg8_map;

    Registers();

    Register16Proxy get16(const std::string& name);
    Register8Proxy get8(const std::string& name);

    bool is8(const std::string& name) const;
    bool is16(const std::string& name) const;

    std::string dump() const;

    // Change tracking API (used internally by proxies)
    void mark_register_change(const std::string& name, uint16_t old_value, uint16_t new_value);
    void mark_flag_change(const std::string& flag_name, bool old_value, bool new_value);
    ChangeSet get_last_changes();  // Returns changes and clears tracking

    // Flag tracking helpers
    void capture_flags();  // Capture current flags state before command
    void check_flag_changes();  // Check which flags changed after command

private:
    ChangeSet m_change_set;
    uint16_t m_captured_flags_value;
};
