import logging
import serial
import io
from time import sleep

# deques are used for sharing of data
from collections import deque

# Use logger for, um, logging...
logging.basicConfig(level=logging.DEBUG)

# Couple of helpful macros used in conversion of strings to ints.
HEX = 16
BIN = 2

# SCALE_FACTOR is multiplied by the normalized signal magnitude, where the full
# scale range is -1 to 1
SCALE_FACTOR = 8.44190

data_buffer = []

class SerialComms():
    ser = serial.Serial()
    ser.baudrate = 1500000
    ser.bytesize = serial.EIGHTBITS
    ser.parity = serial.PARITY_NONE
    ser.stopbits = serial.STOPBITS_ONE
    ser.timeout = 0.5
    ser.port = 'COM5'

    number_of_samples = 0
    sample_rate = 0

    loop_running = False
    stop_loops = False
    accumulated_status = '0b00000000'

    def __init__(self):
        self.input_data_queue = deque(maxlen=1)
        self.decoded_data_queue = deque(maxlen=1)
        self.ser.open()
        self.sio = io.TextIOWrapper(io.BufferedRWPair(self.ser, self.ser), encoding='utf-8')
        self.reset()

    def status(self):
        '''Get the status register
        Calling this will also pick out the decimation rate and update the
        sample rate.'''

        # Consume anything in the input buffer.
        self.sio.read()
        # Write control signal
        self.write('R')
        logging.debug('Message sent: read status register')

        status_table = [self.readline() for kk in range(4)]
        return status_table

    def start_sampling(self):
        '''Get the adc to start sampling.'''
        self.write('S')
        logging.debug('Message sent: Start sampling')

    def write(self, signal):
        '''Send a control signal'''
        self.sio.write(signal)
        self.sio.flush()
        sleep(0.1)

    def readline(self):
        '''Read a line from the buffer.'''
        return self.sio.readline().strip()

    def reset(self):
        self.ser.send_break(0.1)
        logging.debug('Message sent: Break command ')
        self.ser.reset_input_buffer()
        sleep(0.1)

    def setup(self):
        self.write('P')
        logging.debug('Message sent: Setup Signal')


    def acquisition_loop(self):
        self.start_sampling()

        data_buffer = []
        '''Loop that waits for start, collects all the samples and stores result.'''
        for nn in range(int(self.number_of_samples)*6):
            # find the start of the data
            if self.stop_loops:
                logging.info(f'Stop loop encountered')
                return
            cur_line = self.readline()
            if 'start' in cur_line:
                logging.info(f'start line found: "{cur_line}"')
                cur_line = self.readline()
                logging.debug(f'first received message: "{cur_line}"')
                for kk in range(int(self.number_of_samples)*2):
                    if ('stop' in cur_line):
                        break
                    if self.stop_loops:
                        logging.info(f'Stop loop encountered')
                        return
                    try:
                        data_buffer.append(int(cur_line, HEX))
                    except ValueError:
                        logging.error(f'Non Hex message received: "{cur_line}"')
                        # signal not received correctly. retransmit the
                        # same signal
                    # logging.info(f'Current size of data buffer {len(data_buffer)}')
                    #  Write the signal
                    cur_line = self.readline()
                    # logging.debug(f'message received: "{cur_line}"')

                logging.info(f'End line found: "{cur_line}"')

                self.input_data_queue.append(data_buffer)
                return
        logging.error('"start" was never found.')
        return

    def decode_loop(self):
        untangled_buffer = []
        try:
            tangled_buffer = self.input_data_queue.pop()
            self.accumulated_status = '0b0'

            for tangled_sample in tangled_buffer:
                # Convert to 32 bit binary LSB at index 0 for easiest match
                # to the tangled bit order.
                T = format(tangled_sample, '032b')

                #         Bit: |15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
                # Pos in Word: |15|14| 7| 6| 0| 1| 4| 2|10| 8| 9|11|12| 5|13| 3|
                #  Pos in Str: | 0| 1| 8| 9|15|14|11|13| 5| 7| 6| 4| 3|10| 2|12|

                # Reorder Bits:         7       6       5       4       3        2       1      0
                untangled_string = (  T[0]  + T[1]  + T[8]  + T[9]  + T[15] + T[14] + T[11] + T[13]
                                    + T[5]  + T[7]  + T[6]  + T[4]  + T[3]  + T[10] + T[2]  + T[12]
                                    + T[16] + T[17] + T[24] + T[25] + T[31] + T[30] + T[27] + T[29]
                                    + T[21] + T[23] + T[22] + T[20] + T[19] + T[26] + T[18] + T[28] )
                if untangled_string[-6:]  != '010000':
                    pass
                    logging.debug(f'Impossible status, Untangled String: {untangled_string}')
                # logging.debug(f'Untangled String: {untangled_string}')
                self.accumulated_status = format((int(self.accumulated_status, BIN)
                                                      | int(untangled_string[-8:], BIN)), '#010b')
                if untangled_string[0] == '1':
                    # indicates negative in twos complement
                    untangled_buffer.append((int(untangled_string[0:24], BIN)
                                             - (1<<24)) / (2**23-1) * SCALE_FACTOR)
                else:
                    untangled_buffer.append((int(untangled_string[1:24], BIN))
                                            / (2**23 - 1) * SCALE_FACTOR)

            self.decoded_data_queue.append(untangled_buffer)

        except IndexError:
            # indicates input_data_queue is empty, can happen if stop_loops was
            # set true.
            pass

    def modify_sample_properties(self, sample_rate, number_of_samples):

        self.stop_loops = True

        sample_rate_to_decimation_dict = {'2500000': '0',
                                          '1250000': '1',
                                          '625000': '2',
                                          '312500': '3',
                                          '156250': '4',
                                          '78125': '5',
                                          '': '0'}

        number_of_samples_to_MCU_code = {'1024': '0',
                                         '4096': '1',
                                         '8192': '2',
                                         '16384': '3'}

        decimation_rate = sample_rate_to_decimation_dict[sample_rate]
        sample_code = number_of_samples_to_MCU_code[number_of_samples]

        self.reset()
        self.setup()

        # write the decimation rate
        self.write('D')
        self.write(decimation_rate)
        # write the sample_rate
        self.write('L')
        self.write(sample_code)

        logging.debug(f'Message sent: Decimation "D{decimation_rate}, Sampling "L{sample_code}')

        # Update the internal parameters.
        self.sample_rate = int(sample_rate)
        self.number_of_samples = int(number_of_samples)

        status_reg = self.status()

        self.stop_loops = False
        logging.debug("stop loop set to false")

        return status_reg

if __name__ == '__main__':
    '''if this file is run on its own it will perform some tests including
    timing how long a transfer takes '''

    import timeit
    TEST_ITERATIONS = 10

    ser_test = SerialComms()

    ser_test.input_data_queue = deque(maxlen=TEST_ITERATIONS)
    ser_test.decoded_data_queue = deque(maxlen=TEST_ITERATIONS)

    ser_test.modify_sample_properties('156250', '1024')

    print('\n'.join(ser_test.status()))

    ser_test.write('v')
    logging.info(f'Overrange: 0x{ser_test.readline().upper()} default is 0xCCCC')
    ser_test.write('f')
    logging.info(f'Offset: 0x{ser_test.readline().upper()} default is 0x0000')
    ser_test.write('g')
    logging.info(f'Gain: 0x{ser_test.readline().upper()} default is 0xA000')


    logging.info('Acquisition Loop time: {}'.format(
        timeit.timeit(ser_test.acquisition_loop, number=TEST_ITERATIONS) / TEST_ITERATIONS))
    logging.info('Decode Loop time: {}'.format(
        timeit.timeit(ser_test.decode_loop, number=TEST_ITERATIONS) / TEST_ITERATIONS))
    logging.info(f'length of decoded buffers:\n\t{[len(x) for x in list(ser_test.decoded_data_queue)]}')

