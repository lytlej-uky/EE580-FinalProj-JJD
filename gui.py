import serial

ser = serial.Serial('COM6', 9600, timeout=None)
##ser.close()
##ser.open()

while True:
    print(f'press SW1 or SW2')
    s = ser.read()
    s = int.from_bytes(s, byteorder='little') -48
    print(f'You pressed "SW{s}" of TIVA')
    value = input("Please enter B or b: ")
   # print(f'You entered "{value}"')
    ser.write(value.encode())
