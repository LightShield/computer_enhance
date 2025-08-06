from enum import Enum

class Opcode(Enum):
    MOV_REG2REG = 0b100010

    MOV_MEM_AND_ACCUMULATOR = 0b101000

    # MOV_IMMEDIATE2REG = 0b1011xx  # x can be 0 or 1, depending on the width of the immediate value
    MOV_IMMEDIATE2REG_00 = 0b101100
    MOV_IMMEDIATE2REG_01 = 0b101101
    MOV_IMMEDIATE2REG_10 = 0b101110
    MOV_IMMEDIATE2REG_11 = 0b101111

    MOV_IMMEDIATE2REGORMEMORY = 0b110001

    @staticmethod
    def is_mov_immediate2reg(opcode) -> bool:
        return (opcode & 0b111100) == Opcode.MOV_IMMEDIATE2REG_00.value

class Direction(Enum):
    SOURCE_IN_REG_FIELD = 0b0
    DESTINATION_IN_REG_FIELD = 0b1

class Width(Enum):
    BYTE = 0b0
    WORD = 0b1

class Mode(Enum):
    Memory_Mode_No_Displacement = 0b00 
    Memory_Mode_8bit_Displacement = 0b01
    Memory_Mode_16bit_Displacement = 0b10
    Register_Mode_No_Displacement = 0b11 # AKA Register to Register Mode

class RegistersWidthByte(Enum):
    AL = 0b000
    CL = 0b001
    DL = 0b010
    BL = 0b011
    AH = 0b100
    CH = 0b101
    DH = 0b110
    BH = 0b111

class RegistersWidthWord(Enum):
    AX = 0b000
    CX = 0b001
    DX = 0b010
    BX = 0b011
    SP = 0b100
    BP = 0b101
    SI = 0b110
    DI = 0b111

class RegistersModeNon11(Enum):
    BX_SI = 0b000
    BX_DI = 0b001
    BP_SI = 0b010
    BP_DI = 0b011
    SI = 0b100
    DI = 0b101
    BP = 0b110
    BX = 0b111

def GetStringFromRegistersModeNon11(reg):
    if reg == RegistersModeNon11.BX_SI:
        return "BX+SI"
    elif reg == RegistersModeNon11.BX_DI:
        return "BX+DI"
    elif reg == RegistersModeNon11.BP_SI:
        return "BP+SI"
    elif reg == RegistersModeNon11.BP_DI:
        return "BP+DI"
    elif reg == RegistersModeNon11.SI:
        return "SI"
    elif reg == RegistersModeNon11.DI:
        return "DI"
    elif reg == RegistersModeNon11.BP:
        return "BP"
    elif reg == RegistersModeNon11.BX:
        return "BX"
    else:
        raise ValueError(f"Invalid register mode for non-11 mode, got {reg}")
 

def get_registers(reg, width):
    if width == Width.BYTE:
        return RegistersWidthByte(reg)
    elif width == Width.WORD:
        return RegistersWidthWord(reg)
    else:
        raise ValueError("Invalid width for register extraction")