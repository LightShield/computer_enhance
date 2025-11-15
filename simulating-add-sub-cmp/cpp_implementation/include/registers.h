#pragma once
#include <string>
#include <unordered_map>
#include "change_tracking.h"
#include "register_proxy.h"
#include "register_types.h"

struct Registers {
    std::unordered_map<std::string, Register16*> reg16_map;
    std::unordered_map<std::string, uint8_t*> reg8_map;

    Register16 ax, bx, cx, dx, si, di, bp, sp;
    Flags flags;

    Registers();

    Register16Proxy get16(const std::string& name);
    Register8Proxy get8(const std::string& name);

    bool is8(const std::string& name) const;
    bool is16(const std::string& name) const;

    std::string dump() const;

    void mark_register_change(const std::string& name, uint16_t old_value, uint16_t new_value);
    void mark_flag_change(const std::string& flag_name, bool old_value, bool new_value);
    ChangeSet get_last_changes();

    void capture_flags();
    void check_flag_changes();

private:
    ChangeSet m_change_set;
    uint16_t m_captured_flags_value;
};
