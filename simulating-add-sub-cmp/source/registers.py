from dataclasses import dataclass, field

class Register16:
    def __init__(self, value=0):
        self._value = value & 0xFFFF  # 16-bit storage

    @property
    def value(self):
        return self._value

    @value.setter
    def value(self, word):
        # Allow assigning another register directly
        if isinstance(word, Register16) or isinstance(word, Register8View):
            word = word.value
        self._value = word & 0xFFFF

    def __repr__(self):
        return f"0x{self._value:04X} ({self._value})"
    
    def __format__(self, format_spec):
        if format_spec == "":
            return repr(self)
        return format(self.value, format_spec)


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

    def __repr__(self):
        return (f"0x{self._value:04X} ({self._value})")
    
class Register8View:
    """A view of the low or high 8 bits of a 16-bit register."""
    def __init__(self, parent: 'Register16WithLowHigh', part: str):
        assert part in ('low', 'high')
        self._parent = parent
        self._part = part

    @property
    def value(self):
        return getattr(self._parent, self._part)

    @value.setter
    def value(self, val):
        # Allow assigning another register directly
        if isinstance(val, (Register16, Register8View)):
            val = val.value
        setattr(self._parent, self._part, val)

    def __int__(self):
        return self.value

    def __repr__(self):
        v = self.value
        return f"0x{v:02X} ({v})"
    
    def __format__(self, fmt):
        return format(self.value, fmt)


def add_aliases(cls, low_alias, high_alias):
    setattr(cls, low_alias,
            property(fget=lambda self: self.low,
                     fset=lambda self, val: setattr(self, 'low', val)))
    setattr(cls, high_alias,
            property(fget=lambda self: self.high,
                     fset=lambda self, val: setattr(self, 'high', val)))


# Dynamically create AX, BX, CX, DX with aliases using loop over 'a'-'d'
for ch in ['a', 'b', 'c', 'd']:
    reg_name = f"{ch}x"
    low_alias = f"{ch}l"
    high_alias = f"{ch}h"
    cls_name = reg_name.upper()  # AX, BX, CX, DX

    reg_cls = type(cls_name, (Register16WithLowHigh,), {})
    globals()[cls_name] = reg_cls
    add_aliases(reg_cls, low_alias, high_alias)


# Dynamically create other registers without low/high aliases
for reg in ['sp', 'bp', 'si', 'di', 'es', 'ss', 'ds']:
    cls_name = reg.upper()
    reg_cls = type(cls_name, (Register16,), {})
    globals()[cls_name] = reg_cls


@dataclass
class Registers:
    _ax: 'AX' = field(default_factory=lambda: AX())
    _bx: 'BX' = field(default_factory=lambda: BX())
    _cx: 'CX' = field(default_factory=lambda: CX())
    _dx: 'DX' = field(default_factory=lambda: DX())

    _sp: 'SP' = field(default_factory=lambda: SP())
    _bp: 'BP' = field(default_factory=lambda: BP())
    _si: 'SI' = field(default_factory=lambda: SI())
    _di: 'DI' = field(default_factory=lambda: DI())

    _es: 'ES' = field(default_factory=lambda: ES())
    _ss: 'SS' = field(default_factory=lambda: SS())
    _ds: 'DS' = field(default_factory=lambda: DS())

    def __repr__(self):
        regs = (f"AX={self.ax}\nBX={self.bx}\nCX={self.cx}\nDX={self.dx}\n"
                f"SP={self.sp}\nBP={self.bp}\nSI={self.si}\nDI={self.di}\n"
                f"ES={self.es}\nSS={self.ss}\nDS={self.ds}")
        return f"{regs}"
    
    # flags
    carry_flag : bool = False
    parity_flag  : bool = False
    auxiliary_carry_flag  : bool = False
    zero_flag  : bool = False
    sign_flag  : bool = False
    trap_flag  : bool = False
    interrupt_enable_flag  : bool = False
    direction_flag  : bool = False
    overflow_flag  : bool = False


def make_reg_property(reg_name: str):
    private_name = f"_{reg_name}"

    def getter(self):
        return getattr(self, private_name)

    def setter(self, val):
        if isinstance(val, Register16):
            setattr(self, private_name, val)  # allow full replacement
        else:
            getattr(self, private_name).value = val  # assign .value for syntactic sugar

    return property(getter, setter)


# Add properties for registers with syntactic sugar setter/getter
for reg_name in ['ax', 'bx', 'cx', 'dx', 'sp', 'bp', 'si', 'di', 'es', 'ss', 'ds']:
    setattr(Registers, reg_name, make_reg_property(reg_name))


# Add alias properties (al, ah, bl, bh, etc.) on the Registers wrapper class
def add_register_aliases_to_wrapper(wrapper_cls):
    prefix_to_reg = {
        'a': 'ax',
        'b': 'bx',
        'c': 'cx',
        'd': 'dx',
    }
    for prefix, reg_name in prefix_to_reg.items():
        for suffix, part in [('l', 'low'), ('h', 'high')]:
            alias = prefix + suffix

            # getter returns a Register8View object (the live 8-bit view)
            def make_getter(reg_name=reg_name, part=part):
                def getter(self):
                    reg = getattr(self, reg_name)
                    return Register8View(reg, part)
                return getter

            # setter writes directly through the parent register
            def make_setter(reg_name=reg_name, part=part):
                def setter(self, value):
                    reg = getattr(self, reg_name)
                    setattr(reg, part, value)
                return setter

            setattr(wrapper_cls, alias, property(make_getter(), make_setter()))

add_register_aliases_to_wrapper(Registers)


if __name__ == "__main__":
    regs = Registers()
    print(regs)
    regs.ax = 0x1234
    print(f"AX: {regs.ax}, AL: {regs.al}, AH: {regs.ah}")  # note use regs.al instead of regs.ax.al
    regs.bx.bl = 0x56
    regs.bx.bh = 0x78
    regs.bl = 0x9A     # accessing alias on wrapper
    print(f"BX: {regs.bx}, BL: {regs.bl}, BH: {regs.bh}")
    print('----')
    print(regs)
