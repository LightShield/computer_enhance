
import argparse
from decode_enums import Opcode, Direction, Width, Mode, get_registers, RegistersModeNon11, GetStringFromRegistersModeNon11
from binary_stream import BinaryStream

def extract_bits(byte, msb, lsb):
    msb_mask = (1 << msb +1) -1
    extracted_bits = (byte & msb_mask) >> lsb
    print(f"bits after extraction: {((byte & msb_mask) >> lsb):b}")
    return extracted_bits

def get_multi_byte_value(binary_stream, is_16bit):
    displacement_low = binary_stream.read_next_byte()
    displacement_high = ""
    displacement_value = displacement_low

    if is_16bit:
        displacement_high = binary_stream.read_next_byte()
        displacement_value = (displacement_high << 8) | displacement_low
    return displacement_value, displacement_low, displacement_high

def decode_mov_reg2reg_no_displacement(reg, rm, direction, width):
    decoded_reg = get_registers(reg, Width(width))
    decoded_rm = get_registers(rm, Width(width))
    if (direction == Direction.SOURCE_IN_REG_FIELD.value):
        return f"MOV {decoded_rm.name}, {decoded_reg.name}"
    return  f"MOV {decoded_reg.name}, {decoded_rm.name}"

def get_rm_with_displacement(rm):
    return GetStringFromRegistersModeNon11(RegistersModeNon11(rm))

def append_displacement_to_register(decoded_rm, displacement_value):
    if decoded_rm == "DIRECT_ADDRESSING_MODE":
        return f"[{displacement_value}]"
    if displacement_value != 0:
        return f"[{decoded_rm} + {displacement_value}]"
    return f"[{decoded_rm}]"

def decode_mov_reg2reg(binary_stream, instruction_byte_0):
    instruction_byte_1 = binary_stream.read_next_byte()
    direction = extract_bits(instruction_byte_0, 1, 1)  
    width = extract_bits(instruction_byte_0, 0, 0) 
     
    mode = extract_bits(instruction_byte_1, 7, 6)  
    reg = extract_bits(instruction_byte_1, 5, 3)
    rm = extract_bits(instruction_byte_1, 2, 0)

    print(f"Decoded opcode: {Opcode.MOV_REG2REG.value}, Direction: {direction}, Width: {width}, Mode: {mode}")
    print(f"Decoded MOV instruction: Direction={Direction(direction).name}, Width={Width(width).name}, Mode={Mode(mode).name}")

    if mode == Mode.Register_Mode_No_Displacement.value:
        print(f"Raw instruction bits: {instruction_byte_0:b} {instruction_byte_1:b}")
        return decode_mov_reg2reg_no_displacement(reg, rm, direction, width)
    
    decoded_reg = get_registers(reg, Width(width))
    decoded_rm = get_rm_with_displacement(rm)

    is_displacement_16bit = mode == Mode.Memory_Mode_16bit_Displacement.value
    if mode == Mode.Memory_Mode_No_Displacement.value:
        print(f"Raw instruction bits: {instruction_byte_0:b} {instruction_byte_1:b}")
        if rm != 0b110:  
            return return_decoded_mov_reg2reg_direction(decoded_reg, f"[{decoded_rm}]", direction)
        # else - rm == 0b110 - Special case for BP\BX register - "Direct addressing mode" - there is a displacement, counter to what the "mode" suggests
        decoded_rm = "DIRECT_ADDRESSING_MODE"
        is_displacement_16bit = True  # we will read displacement as 16bit,
    
    displacement_value, displacement_low, displacement_high = get_multi_byte_value(binary_stream, is_displacement_16bit)
    
    decoded_rm = append_displacement_to_register(decoded_rm, displacement_value)

    print (f"Raw instruction bits: {instruction_byte_0:b} {instruction_byte_1:b} {displacement_low:b} {displacement_high if displacement_high else '' }")
    print(f"Displacement mode detected: {Mode(mode).name}, Displacement value: {displacement_value}")
    return return_decoded_mov_reg2reg_direction(decoded_reg, decoded_rm, direction)

def return_decoded_mov_reg2reg_direction(decoded_reg, decoded_rm, direction):
    if direction == Direction.SOURCE_IN_REG_FIELD.value:
        return f"MOV {decoded_rm}, {decoded_reg.name}"
    return f"MOV {decoded_reg.name}, {decoded_rm}"

def decode_mov_mem_and_accumulator(binary_stream, instruction_byte_0):
    direction = extract_bits(instruction_byte_0, 1, 1)  
    width = extract_bits(instruction_byte_0, 0, 0) 

    address_low = binary_stream.read_next_byte()
    address_high = ""
    address_value = address_low
    if width == Width.WORD.value:
        address_high = binary_stream.read_next_byte()
        address_value = (address_high << 8) | address_low
     
    address_value = f"[{address_value}]" 
    register = get_registers(reg=000, width=Width(width)) # hard codewd for accumulator register (AL or AX)
    print(f"Raw instruction bits: {instruction_byte_0:08b} {address_low:08b} {address_high if address_high else ''}")
    if direction == Direction.SOURCE_IN_REG_FIELD.value:
        return f"MOV {address_value}, {register.name}"
    return f"MOV {register.name}, {address_value}"

def decode_move_immediate2regormemory(binary_stream, instruction_byte_0):
    instruction_byte_1 = binary_stream.read_next_byte()
    direction = extract_bits(instruction_byte_0, 1, 1)  
    width = extract_bits(instruction_byte_0, 0, 0) 
     
    mode = extract_bits(instruction_byte_1, 7, 6)  
    reg = extract_bits(instruction_byte_1, 5, 3)
    rm = extract_bits(instruction_byte_1, 2, 0)

    if reg != 0b000:
        raise RuntimeError(f"Invalid register field in MOV immediate to register or memory instruction: {reg:03b}")
    if direction != Direction.DESTINATION_IN_REG_FIELD.value:
        raise RuntimeError(f"Invalid direction field in MOV immediate to register or memory instruction: {direction}")
    

    decoded_rm = get_rm_with_displacement(rm)

    is_displacement_16bit = mode == Mode.Memory_Mode_16bit_Displacement.value
    displacement, displacement_low, displacement_high = get_multi_byte_value(binary_stream, is_displacement_16bit)

    if mode == Mode.Memory_Mode_No_Displacement.value or mode == Mode.Register_Mode_No_Displacement.value:
        data, data_low, data_high  = 0, '', ''
    else:
        is_data_16bit = width == Width.WORD.value
        data, data_low, data_high = get_multi_byte_value(binary_stream, is_data_16bit)

    if data != 0:
        # we need to reverse the order of data and displacement - base on output of example. didn't find any documentation about it
        data, displacement = displacement, data
        is_displacement_16bit, is_data_16bit = is_data_16bit, is_displacement_16bit
    decoded_rm = append_displacement_to_register(decoded_rm, data)

    print(f"Raw instruction bits: {instruction_byte_0:8b} {instruction_byte_1:8b} {displacement_low:8b} {displacement_high if displacement_high else '' } {data_low} {data_high}")
    print(f"Decoded MOV immediate to register or memory instruction: Direction={Direction(direction).name}, Width={Width(width).name}, Mode={Mode(mode).name}, Decoded_rm = {decoded_rm}, Displacement={displacement}, Data={data}")

    if not is_displacement_16bit and displacement < 0xFF:
        displacement = f"byte, {displacement}"
    else:
        displacement = f"word, {displacement}"
    return  f"MOV {decoded_rm} {displacement}"

def decode_mov_immediate2reg(instruction_byte_0, binary_stream):
    reg = extract_bits(instruction_byte_0, 2, 0)
    width = extract_bits(instruction_byte_0, 3, 3)
    immediate_value = binary_stream.read_next_byte()

    if width == Width.WORD.value:
        immediate_high = binary_stream.read_next_byte()
        immediate_value |= (immediate_high << 8) 

    decoded_reg = get_registers(reg, Width(width))
    return f"MOV {decoded_reg.name}, {immediate_value}"

def decode_instruction(binary_stream, instruction_byte_0):
    opcode = extract_bits(instruction_byte_0, 7, 2)
    if Opcode.is_mov_immediate2reg(opcode):
        print("MOV immediate to register instruction detected")
        return decode_mov_immediate2reg(instruction_byte_0, binary_stream)
        
    match opcode:
        case Opcode.MOV_REG2REG.value:
            return decode_mov_reg2reg(binary_stream, instruction_byte_0)
        case Opcode.MOV_IMMEDIATE2REGORMEMORY.value:
            return decode_move_immediate2regormemory(binary_stream, instruction_byte_0)
        case Opcode.MOV_MEM_AND_ACCUMULATOR.value:
            return decode_mov_mem_and_accumulator(binary_stream, instruction_byte_0)
        case _:
            raise RuntimeError(f"Unhandled opcode: {opcode:06b}")   
   

def main():
    parser = argparse.ArgumentParser(description="Manual disassembler for 8086 instructions.")
    parser.add_argument("filename", type=str, help="The binary file to disassemble.")
    args = parser.parse_args()

    binary_stream = BinaryStream(args.filename)
    with open("decoded_instructions.txt", "a") as f:
        while True:
            try:
                next_byte = binary_stream.read_next_byte()
                if next_byte is None:
                    print("No more bytes to read.")
                    break
                decoded_instruction = decode_instruction(binary_stream, next_byte)
                print("decoded instruction: ", decoded_instruction)
                f.write(decoded_instruction + "\n")
            except StopIteration:
                print("End of file reached - No more bytes to read.")
                break
            except RuntimeError as e:
                print(f"Error decoding instruction: {e}")
                break

if __name__ == "__main__":
    main()