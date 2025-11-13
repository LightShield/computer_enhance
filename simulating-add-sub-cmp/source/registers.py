from dataclasses import dataclass, field

# -------------------------
# Shared arithmetic behavior
# -------------------------
class RegisterArithmeticMixin:
    """Mixin for shared arithmetic/comparison behavior of register types."""
    _bit_mask = 0xFFFF  # subclasses override this

    def _to_int(self, other):
        if isinstance(other, (Register16, Register8View)):
            return other.value
        return int(other)

    def __add__(self, other):
        return (self.value + self._to_int(other)) & self._bit_mask

    def __sub__(self, other):
        return (self.value - self._to_int(other)) & self._bit_mask

    def __eq__(self, other):
        return self.value == self._to_int(other)

    def __int__(self):
        return self.value

    def __format__(self, fmt):
        return format(self.value, fmt)


# -------------------------
# Base register classes
# -------------------------
class Register16(RegisterArithmeticMixin):
    _bit_mask = 0xFFFF

    def __init__(self, value=0, name: str | None = None):
        self._value = value & self._bit_mask
        self._name = name

    @property
    def value(self):
        return self._value

    @value.setter
    def value(self, word):
        if isinstance(word, (Register16, Register8View)):
            word = word.value
        self._value = word & self._bit_mask

    def __repr__(self):
        return f"0x{self._value:04X} ({self._value})"

    @property
    def name(self):
        return self._name


class Register16WithLowHigh(Register16):
    @property
    def low(self):
        return self._value & 0x00FF

    @low.setter
    def low(self, val):
        self._value = (self._value & 0xFF00) | (val & 0x00FF)

    @property
    def high(self):
        return (self._value & 0xFF00) >> 8

    @high.setter
    def high(self, val):
        self._value = (self._value & 0x00FF) | ((val & 0x00FF) << 8)


class Register8View(RegisterArithmeticMixin):
    """A view of the low or high 8 bits of a 16-bit register."""
    _bit_mask = 0xFF

    def __init__(self, parent: 'Register16WithLowHigh', part: str, name: str | None = None):
        assert part in ('low', 'high')
        self._parent = parent
        self._part = part
        self._name = name

    @property
    def value(self):
        return getattr(self._parent, self._part)

    @value.setter
    def value(self, val):
        if isinstance(val, (Register16, Register8View)):
            val = val.value
        setattr(self._parent, self._part, val)

    def __repr__(self):
        v = self.value
        if self._name:
            return f"{self._name}=0x{v:02X} ({v})"
        return f"0x{v:02X} ({v})"

    @property
    def name(self):
        return self._name


# -------------------------
# Dynamic register creation
# -------------------------
def add_aliases(cls, low_alias, high_alias):
    setattr(cls, low_alias,
            property(fget=lambda self: self.low,
                     fset=lambda self, val: setattr(self, 'low', val)))
    setattr(cls, high_alias,
            property(fget=lambda self: self.high,
                     fset=lambda self, val: setattr(self, 'high', val)))


# AX, BX, CX, DX — have low/high
for ch in ['a', 'b', 'c', 'd']:
    reg_name = f"{ch}x"
    low_alias = f"{ch}l"
    high_alias = f"{ch}h"
    cls_name = reg_name.upper()
    reg_cls = type(cls_name, (Register16WithLowHigh,), {})
    globals()[cls_name] = reg_cls
    add_aliases(reg_cls, low_alias, high_alias)

# SP, BP, SI, DI, ES, SS, DS — simple 16-bit registers
for reg in ['sp', 'bp', 'si', 'di', 'es', 'ss', 'ds']:
    cls_name = reg.upper()
    reg_cls = type(cls_name, (Register16,), {})
    globals()[cls_name] = reg_cls


# -------------------------
# Flags register
# -------------------------
class FlagsRegister:
    def __init__(self):
        self._carry_flag = False
        self._parity_flag = False
        self._auxiliary_carry_flag = False
        self._zero_flag = False
        self._sign_flag = False
        self._trap_flag = False
        self._interrupt_enable_flag = False
        self._direction_flag = False
        self._overflow_flag = False

    def __repr__(self):
        return (f"CF={int(self._carry_flag)} PF={int(self._parity_flag)} "
                f"AF={int(self._auxiliary_carry_flag)} ZF={int(self._zero_flag)} "
                f"SF={int(self._sign_flag)} TF={int(self._trap_flag)} "
                f"IF={int(self._interrupt_enable_flag)} DF={int(self._direction_flag)} "
                f"OF={int(self._overflow_flag)}")

    def print_changed_flags_from_prev_state(self, prev_flags: 'FlagsRegister'):
        changes = []
        for name in ["carry", "parity", "auxiliary_carry", "zero", "sign",
                     "trap", "interrupt_enable", "direction", "overflow"]:
            field = f"_{name}_flag"
            if getattr(self, field) != getattr(prev_flags, field):
                changes.append(f"{field[1:3].upper()}={int(getattr(self, field))}")
        return " ".join(changes)


# -------------------------
# Register set
# -------------------------
@dataclass
class Registers:
    _ax: 'AX' = field(default_factory=lambda: AX(name='AX'))
    _bx: 'BX' = field(default_factory=lambda: BX(name='BX'))
    _cx: 'CX' = field(default_factory=lambda: CX(name='CX'))
    _dx: 'DX' = field(default_factory=lambda: DX(name='DX'))

    _sp: 'SP' = field(default_factory=lambda: SP(name='SP'))
    _bp: 'BP' = field(default_factory=lambda: BP(name='BP'))
    _si: 'SI' = field(default_factory=lambda: SI(name='SI'))
    _di: 'DI' = field(default_factory=lambda: DI(name='DI'))

    _es: 'ES' = field(default_factory=lambda: ES(name='ES'))
    _ss: 'SS' = field(default_factory=lambda: SS(name='SS'))
    _ds: 'DS' = field(default_factory=lambda: DS(name='DS'))

    _flags: 'FlagsRegister' = field(default_factory=FlagsRegister)

    def __repr__(self):
        parts = []
        for name in ['ax', 'bx', 'cx', 'dx', 'sp', 'bp', 'si', 'di', 'es', 'ss', 'ds']:
            reg_obj = getattr(self, name)
            reg_name = getattr(reg_obj, 'name', None) or name.upper()
            parts.append(f"{reg_name}={reg_obj}")
        return "\n".join(parts)


# -------------------------
# Register properties (syntactic sugar)
# -------------------------
def make_reg_property(reg_name: str):
    private_name = f"_{reg_name}"

    def getter(self):
        return getattr(self, private_name)

    def setter(self, val):
        if isinstance(val, Register16):
            setattr(self, private_name, val)
        else:
            getattr(self, private_name).value = val

    return property(getter, setter)


for reg_name in ['ax', 'bx', 'cx', 'dx', 'sp', 'bp', 'si', 'di', 'es', 'ss', 'ds']:
    setattr(Registers, reg_name, make_reg_property(reg_name))


def add_register_aliases_to_wrapper(wrapper_cls):
    prefix_to_reg = {'a': 'ax', 'b': 'bx', 'c': 'cx', 'd': 'dx'}
    for prefix, reg_name in prefix_to_reg.items():
        for suffix, part in [('l', 'low'), ('h', 'high')]:
            alias = prefix + suffix

            def make_getter(reg_name=reg_name, part=part, alias=alias):
                def getter(self):
                    # name the 8-bit view using the alias in uppercase (e.g., 'AL')
                    return Register8View(getattr(self, reg_name), part, name=alias.upper())
                return getter

            def make_setter(reg_name=reg_name, part=part):
                def setter(self, value):
                    setattr(getattr(self, reg_name), part, value)
                return setter

            setattr(wrapper_cls, alias, property(make_getter(), make_setter()))

add_register_aliases_to_wrapper(Registers)


# -------------------------
# Example usage
# -------------------------
if __name__ == "__main__":
    regs = Registers()
    regs.ax = 0x1234
    print(f"AX: {regs.ax}, AL: {regs.al}, AH: {regs.ah}")
    regs.bx.bl = 0x56
    regs.bx.bh = 0x78
    regs.bl = 0x9A
    print(f"BX: {regs.bx}, BL: {regs.bl}, BH: {regs.bh}")
