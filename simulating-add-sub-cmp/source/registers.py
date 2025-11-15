class Register:
    """Base class for all registers (8-bit or 16-bit)."""
    def __init__(self, name: str, bits: int = 16):
        self.name = name
        self.bits = bits
        self._value = 0

    @property
    def value(self):
        return self._value

    @value.setter
    def value(self, val):
        mask = (1 << self.bits) - 1
        self._value = val & mask

    def __repr__(self):
        return f"{self.name.upper()}"

    # Arithmetic and comparison operator overloading
    def __add__(self, other):
        val = other.value if isinstance(other, Register) else other
        return (self.value + val) & ((1 << self.bits) - 1)

    def __sub__(self, other):
        val = other.value if isinstance(other, Register) else other
        return (self.value - val) & ((1 << self.bits) - 1)

    def __eq__(self, other):
        val = other.value if isinstance(other, Register) else other
        return self.value == val

    def __lt__(self, other):
        val = other.value if isinstance(other, Register) else other
        return self.value < val


class Register8View(Register):
    """Represents AH, AL, BH, BL, etc., views into a 16-bit register."""
    def __init__(self, parent: 'Register', high: bool):
        name = parent.name[0] + ('H' if high else 'L')
        super().__init__(name, bits=8)
        self.parent = parent
        self.high = high

    @property
    def value(self):
        if self.high:
            return (self.parent.value >> 8) & 0xFF
        return self.parent.value & 0xFF

    @value.setter
    def value(self, val):
        val &= 0xFF
        if self.high:
            self.parent.value = (self.parent.value & 0x00FF) | (val << 8)
        else:
            self.parent.value = (self.parent.value & 0xFF00) | val


class Flags:
    """Full 8086-style flags register."""
    def __init__(self):
        self.CF = 0  # Carry
        self.PF = 0  # Parity
        self.AF = 0  # Auxiliary carry
        self.ZF = 0  # Zero
        self.SF = 0  # Sign
        self.TF = 0  # Trap
        self.IF = 0  # Interrupt enable
        self.DF = 0  # Direction
        self.OF = 0  # Overflow

    def __repr__(self):
        flags = []
        for f in ["CF", "PF", "AF", "ZF", "SF", "TF", "IF", "DF", "OF"]:
            if getattr(self, f):
                flags.append(f)
        return "[" + " ".join(flags) + "]" if flags else "[ ]"

    def reset(self):
        for attr in vars(self):
            setattr(self, attr, 0)


class Registers:
    """All general-purpose registers and flags."""
    def __init__(self):
        # 16-bit general purpose registers
        self.ax = Register('ax')
        self.bx = Register('bx')
        self.cx = Register('cx')
        self.dx = Register('dx')
        self.si = Register('si')
        self.di = Register('di')
        self.bp = Register('bp')
        self.sp = Register('sp')

        # 8-bit views
        self.ah = Register8View(self.ax, True)
        self.al = Register8View(self.ax, False)
        self.bh = Register8View(self.bx, True)
        self.bl = Register8View(self.bx, False)
        self.ch = Register8View(self.cx, True)
        self.cl = Register8View(self.cx, False)
        self.dh = Register8View(self.dx, True)
        self.dl = Register8View(self.dx, False)

        # Flags
        self.flags = Flags()

    def __repr__(self):
        return (f"AX={self.ax.value:04X} BX={self.bx.value:04X} "
                f"CX={self.cx.value:04X} DX={self.dx.value:04X} "
                f"SI={self.si.value:04X} DI={self.di.value:04X} "
                f"BP={self.bp.value:04X} SP={self.sp.value:04X} {self.flags}")

    def reset(self):
        for attr, val in vars(self).items():
            if isinstance(val, Register):
                val.value = 0
        self.flags.reset()
