from dataclasses import dataclass, field

class Register16:
    def __init__(self, value=0):
        self._value = value & 0xFFFF  # 16-bit storage

    @property
    def value(self):
        return self._value

    @value.setter
    def value(self, word):
        self._value = word & 0xFFFF

    def __repr__(self):
        return f"0x{self._value:04X} ({self._value})"
    
    def __format__(self, format_spec):
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
    ax: 'AX' = field(default_factory=lambda: AX())
    bx: 'BX' = field(default_factory=lambda: BX())
    cx: 'CX' = field(default_factory=lambda: CX())
    dx: 'DX' = field(default_factory=lambda: DX())

    sp: 'SP' = field(default_factory=lambda: SP())
    bp: 'BP' = field(default_factory=lambda: BP())
    si: 'SI' = field(default_factory=lambda: SI())
    di: 'DI' = field(default_factory=lambda: DI())

    es: 'ES' = field(default_factory=lambda: ES())
    ss: 'SS' = field(default_factory=lambda: SS())
    ds: 'DS' = field(default_factory=lambda: DS())

    def __repr__(self):
        regs = (f"AX={self.ax}\nBX={self.bx}\nCX={self.cx}\nDX={self.dx}\n"
                f"SP={self.sp}\nBP={self.bp}\nSI={self.si}\nDI={self.di}\n"
                f"ES={self.es}\nSS={self.ss}\nDS={self.ds}")
        return f"{regs}"
    
if __name__ == "__main__":
    regs = Registers()
    print(regs)
    regs.ax.value = 0x1234
    print(f"AX: {regs.ax}, AL: {regs.ax.al}, AH: {regs.ax.ah}")
    regs.bx.bl = 0x56
    regs.bx.bh = 0x78
    print(f"BX: {regs.bx}, BL: {regs.bx.bl}, BH: {regs.bx.bh}")
    print('----')
    print(regs)
