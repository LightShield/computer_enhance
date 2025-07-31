
# A class that recieves a file name and lets you read 1 byte at a time
class BinaryStream():
    def __init__(self, filename):
        self.filename = filename
        self.file = open(self.filename, 'rb')

    def __enter__(self):
        self.file = open(self.filename, 'rb')
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        if self.file:
            self.file.close()

    def read_next_byte(self):
        byte = self.file.read(1)
        if not byte:
            raise StopIteration("End of file reached")
        return byte[0]
