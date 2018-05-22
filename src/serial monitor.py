import serial

mcu = serial.Serial()

mcu.baudrate = 2000000
mcu.bytesize = serial.EIGHTBITS
mcu.parity = serial.PARITY_NONE
mcu.stopbits = serial.STOPBITS_ONE
mcu.timeout = 0.005
mcu.port = 'COM5'
mcu.open()


cpu = serial.Serial()

cpu.apply_settings(mcu.get_settings());
cpu.port = 'COM2'
cpu.open()

while(1):
    rxmcu = mcu.readline()
    rxcpu = cpu.readline()

    if rxmcu != b'':
        cpu.write(rxmcu)
        print(f'MCU: {rxmcu}')
    if rxcpu != b'':
        mcu.write(rxcpu)
        print(f'CPU: {rxcpu}')

