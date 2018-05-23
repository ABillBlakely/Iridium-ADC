import logging
import serial
import io
from time import sleep

from multiprocessing import Process, Queue, Pipe

# deques are used for thread safe sharing of data
from collections import deque

# Use logger for, um, logging...
logging.basicConfig(level=logging.INFO)

# Couple of helpful macros used in conversion of strings to ints.
HEX = 16
BIN = 2

# input_data_queue = deque(maxlen=3)
# decoded_data_queue = deque(maxlen=3)
stop_thread = 0
# Ok Heres the idea: create thread that continually receives the data and stores each array in a buffer.

data_buffer = []

class SerialComms():
    ser = serial.Serial()
    ser.baudrate = 1500000
    ser.bytesize = serial.EIGHTBITS
    ser.parity = serial.PARITY_NONE
    ser.stopbits = serial.STOPBITS_ONE
    ser.timeout = 0.5
    ser.port = 'COM5'

    number_of_samples = 8192
    sample_rate = 78125
    decimation_to_sample_rate_map = {'1' :2500000,
                                     '2' :1250000,
                                     '4' : 625000,
                                     '8' : 312500,
                                     '16': 156250,
                                     '32':  78125,
                                     '': 0,
                                     }

    stop_loops = False
    accumulated_status = '0b00000000'
    decimation_rate = '32'

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

        status_table = [self.readline() for kk in range(3)]
        self.decimation_rate = status_table[-1][-10:].strip('| ')
        self.sample_rate = self.decimation_to_sample_rate_map[self.decimation_rate]
        return status_table

    def start_sampling(self):
        '''Get the adc to start sampling.'''
        self.write('S')

    def write(self, signal):
        '''Send a control signal'''
        self.sio.write(signal)
        self.sio.flush()

    def readline(self):
        '''Read a line from the buffer.'''
        return self.sio.readline().strip()

    def reset(self):
        self.ser.send_break(0.5)
        sleep(0.1)

    def setup(self):
        self.write('P')

    def acquisition_loop(self):
        self.start_sampling()

        data_buffer = []
        '''Loop that waits for start, collects all the samples and stores result.'''
        for nn in range(self.number_of_samples*2):
            # find the start of the data
            cur_line = self.readline()
            if 'start' in cur_line:
                logging.info(f'start line found: "{cur_line}"')
                cur_line = self.readline()
                logging.debug(f'first received message: "{cur_line}"')
                while ('stop' not in cur_line ):
                    if self.stop_loops:
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
                    logging.debug(f'message received: "{cur_line}"')

                self.input_data_queue.append(data_buffer)
                return
        logging.error('"start" was never found.')
        return

    def decode_loop(self):
        untangled_buffer = []
        try:
            tangled_buffer = self.input_data_queue.pop()
            self.accumulated_status = '0b0'
            # This loop could probably be sped up considerably using map():
            for tangled_sample in tangled_buffer:
                # Convert to 32 bit binary LSB at index 0 for easiest match
                # to the tangled bit order.
                T = format(tangled_sample, '032b')
                #         Bit: | 15 | 14 | 13 | 12 | 11 | 10 |  9 |  8 |  7 |  6 |  5 |  4 |  3 |  2 |  1 |  0 |
                # Pos in Word: | 15 | 14 |  7 |  6 |  0 |  1 |  4 |  2 | 10 |  8 |  9 | 11 | 12 |  5 | 13 |  3 |
                #  Pos in Str: |  0 |  1 |  8 |  9 | 15 | 14 | 11 | 13 |  5 |  7 |  6 |  4 |  3 | 10 |  2 | 12 |

                # Reorder Bits:        8       7       6       5       4       3      2      1      0
                untangled_string = (  T[0]  + T[1]  + T[8]  + T[9]  + T[15] + T[14] + T[11] + T[13]
                                    + T[5]  + T[7]  + T[6]  + T[4]  + T[3]  + T[10]  + T[2]  + T[12]
                                    + T[16] + T[17] + T[24] + T[25] + T[31] + T[30] + T[27] + T[29]
                                    + T[21] + T[23] + T[22] + T[20] + T[19] + T[26] + T[18] + T[28] )
                if untangled_string[-6:]  != '010000':
                     logging.debug(f'Untangled String: {untangled_string}')
                self.accumulated_status = format((int(self.accumulated_status, BIN) | int(untangled_string[-8:], BIN)), '#010b')
                if untangled_string[0] == '1':
                    # indicates negative in twos complement
                    untangled_buffer.append((int(untangled_string[0:24], BIN) - (1<<24)) / (2**23-1) * 12)
                else:
                    untangled_buffer.append((int(untangled_string[1:24], BIN)) / (2**23 - 1) * 12)
                # print(f'{untangled_buffer[-1]}')
            self.decoded_data_queue.append(untangled_buffer)

        except IndexError:
            # indicates input_data_queue is empty
            pass
    def change_decimation_rate(self, rate):
        self.stop_loops = True
        sleep(0.5)
        self.reset()
        self.setup()
        self.write('D' + rate)
        self.stop_loops = False

if __name__ == '__main__':
    import timeit
    TEST_ITERATIONS = 10

    ser_test = SerialComms()

    ser_test.input_data_queue = deque(maxlen=TEST_ITERATIONS)
    ser_test.decoded_data_queue = deque(maxlen=TEST_ITERATIONS)

    ser_test.reset()
    ser_test.setup()
    print('\n'.join(ser_test.status()))
    print(ser_test.decimation_rate)

    # ser_test.start_sampling()

    logging.info('Acquisition Loop time: {}'.format(
        timeit.timeit(ser_test.acquisition_loop, number=TEST_ITERATIONS) / TEST_ITERATIONS))
    logging.info('Decode Loop time: {}'.format(
        timeit.timeit(ser_test.decode_loop, number=TEST_ITERATIONS) / TEST_ITERATIONS))
    logging.info(f'length of decoded buffers:\n\t{[len(x) for x in list(ser_test.decoded_data_queue)]}')

    # for kk in range(200):
    #     try:
    #         ser_test.acquisition_loop()
    #         ser_test.decode_loop()
    #         print(ser_test.decoded_data_queue.pop()[:5])
    #     except IndexError:
    #         print("data queue empty")

    # ser_test.reset()
    # print('\n'.join(ser_test.status()))

    # Only uncomment if you change the acquisition and decode
    # ser_test.acquisition_loop()
    # ser_test.decode_loop()
    # print(ser_test.decoded_data_queue[0])
    # print(ser_test.accumulated_status)

