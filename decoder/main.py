import argparse
import sys
from decoder import Decoder
from binary_stream import BinaryStream

def main():
    parser = argparse.ArgumentParser(description="8086 Decoder CLI")
    parser.add_argument("filename", help="Binary file to decode")
    args = parser.parse_args()

    stream = BinaryStream(args.filename)
    decoder = Decoder(stream)

    while stream.has_more_bytes():
        try:
            instr = decoder.decode_next()
            if instr:
                print(instr)
        except EOFError:
            break
        except Exception as e:
            print(f"Error: {e}")
            break

if __name__ == "__main__":
    main()
