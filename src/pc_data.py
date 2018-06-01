import logging
import serial
import io
import dash
import dash_core_components as dcc
import dash_html_components as html
import dash.dependencies as dd
import dash.exceptions as de
from adc_comms import SerialComms

# deques are used for thread safe sharing of data
from collections import deque
import numpy as np
from time import sleep

# Initialization
adc = SerialComms()

# # Set default decimation rate
# adc.modify_sample_properties('156250', '16384')


# # Pull some information
# adc.write('v')
# logging.info(f'Overrange: 0x{adc.readline().upper()} default is 0xCCCC')
# adc.write('f')
# logging.info(f'Offset: 0x{adc.readline().upper()} default is 0x0000')
# adc.write('g')
# logging.info(f'Gain: 0x{adc.readline().upper()} default is 0xA000')

# Initialize various arrays, global arrays are used for preserving data between
# function invocations.
magnitude = [0]
sample_index = [0]
freq_mag = [1e-24]
freq_mag_history = deque(maxlen=1)
freq_axis = [0]

# Define the refresh interval, determined empirically. it would be better if
# this could retrigger on the acquisition loop callback completion, but I can
# only get this to function off a timer so here we are.
update_period_ms = {'1024': 900,
                    '4096': 2000,
                    '8192': 2000,
                    '16384': 2000 }

# This array constructs the window functions used.
# It interacts with the window-functions id in the dash app.
window_map = {}

# Create the dash app object
app = dash.Dash()

# This is the GUI layout.
app.layout = html.Div(id='Body',
    children=
    [
        # Page/Project title
        html.H1(id='title',
            children="Iridium ADC"
        ),

        # Insert a blank line
        html.P(''),

        # Controls to change sampling properties
        html.Div(id='sampling-properties-container',
            style={'width': '40%'},
            children=
            [
                html.P(''),
                html.Label('Sample Rate Selection:'),
                dcc.Dropdown(id='sample-rate',
                    options=
                    [
                        # 'label' is the sample rate, 'value' corresponds to the
                        # corresponding decimation rate register setting.

                        # higher rates are not supported yet
                        # {'label':'2.5 MSPS', 'value': '2500000'},
                        # {'label':'1.25 MSPS', 'value': '1250000'},
                        # {'label':'625 kSPS', 'value': '625000'},
                        {'label':'312.5 kSPS', 'value': '312500'},
                        {'label':'156.25 kSPS', 'value': '156250'},
                        {'label':'78.125 kSPS', 'value': '78125'},
                    ],
                    value='78125',
                ),

                html.P(''),
                html.Label('Number of samples:'),
                dcc.Dropdown(id='number-of-samples',
                    options=
                    [
                        {'label': '1024', 'value': '1024'},
                        {'label': '4096', 'value': '4096'},
                        {'label': '8192', 'value': '8192'},
                        {'label': '16384', 'value': '16384'},
                    ],
                    value='16384',
                ),

                html.P(''),
                html.Label('Window:'),
                dcc.Dropdown(id='window-functions',
                    options=
                    [
                        {'label':'Rect', 'value':'rect'},
                        {'label':'Blackman', 'value':'blackman'},
                        {'label':'Kaiser 5', 'value':'kaiser5'},
                        {'label':'Kaiser 7', 'value':'kaiser7'},
                    ],
                    value='kaiser5',
                ),

                html.P(''),
                html.Label('Number of averages: '),
                dcc.Input(id='number-of-averages',
                    placeholder='Enter a number',
                    type='number',
                    value=10,
                    pattern='[0-9]+',
                    min=2,
                ),

                html.P(''),

                html.Div(id='control-buttons',
                    style={'column':3},
                    children=
                    [
                        # Button to turn graph updating on or off.
                        html.Button(id='graph-update-button',
                            n_clicks=0,
                            accessKey='p'
                        ),


                        # Button to turn frequency domain magnitude averaging on or off.
                        html.Button(id='freq-average-button',
                            n_clicks=0,
                        ),

                        # Button to toggle time domain data in raw form or windowed form.
                        html.Button(id='show-windowed-button',
                            n_clicks=0,
                        ),
                    ]),
            ],
        ),
        html.Div(style={'column':2},
            children=
            [
                # The time domain graph object.
                dcc.Graph(id='time-domain-graph'),

                # The frequency domain graph object.
                dcc.Graph(id='freq-domain-graph'),
            ]
        ),
        # Interval object triggers the data acquisition
        dcc.Interval(id='update-timer',
            interval=4000,
            n_intervals=0
        ),

        # Accumulated status indicates if the sample contained overrange events.
        # This is returned from the acquisition loop and triggers the update of
        # the graph objects.
        html.Div(id='accumulated-status',
            style={'display': 'none'},
            children=''
        ),

        html.Div(id='placeholder',
            style={'display': 'none'}
        ),
    ])


@app.callback(
    dd.Output('placeholder', 'children'),
    [dd.Input('sample-rate', 'value'),
     dd.Input('number-of-samples', 'value')])
def update_properties(sample_rate, number_of_samples):
    '''Send a control signal to the MCU to change the sample rate or number
    of samples'''
    global window_map, freq_axis

    status_reg_f = '\n'.join(adc.modify_sample_properties(sample_rate, number_of_samples))
    # rebuild the window map
    window_map = {'rect': [1] * adc.number_of_samples,
                  'blackman': np.blackman(adc.number_of_samples),
                  'kaiser5': np.kaiser(adc.number_of_samples, 5 * np.pi),
                  'kaiser7': np.kaiser(adc.number_of_samples, 7 * np.pi),}
    # Get, format, and log the status register.
    print(status_reg_f)

    freq_axis = np.fft.rfftfreq(n=adc.number_of_samples, d=1/adc.sample_rate)

    return ''


@app.callback(
    dd.Output('update-timer', 'interval'),
    [dd.Input('graph-update-button', 'n_clicks'),
     dd.Input('number-of-samples', 'value')])
def toggle_graph_update(n_clicks, number_of_samples):
    '''Prevent the graphs from updating by setting a really long interval time.'''
    if (n_clicks % 2 == 0):
        return update_period_ms[number_of_samples]
    else:
        # Setting disable is not working so instead just set a ridiculously long
        # interval. In this case about 317 years.
        return 10e12

@app.callback(
    dd.Output('graph-update-button', 'children'),
    [dd.Input('graph-update-button', 'n_clicks')])
def update_graph_button(n_clicks):
    '''Update the text on the graph update button'''
    if (n_clicks % 2 == 0):
        return 'Graph updates are ON'
    else:
        return 'Graph updates are OFF'


@app.callback(
    dd.Output('freq-average-button', 'children'),
    [dd.Input('freq-average-button', 'n_clicks')])
def freq_average_button(n_clicks):
    '''Update text on button, the actual effect of the button is handled in
    the frequency domain graph callback.'''
    if (n_clicks % 2 == 0):
        return 'Freq domain averaging OFF'
    else:
        return 'Freq domain averaging ON'


@app.callback(
    dd.Output('show-windowed-button', 'children'),
    [dd.Input('show-windowed-button', 'n_clicks')])
def show_windowed_button(n_clicks):
    '''Update text on button, the actual effect is handled in the time domain
    update callback'''
    if (n_clicks % 2 == 0):
        return 'Time domain window OFF'
    else:
        return 'Time domain window ON'


@app.callback(
    dd.Output('accumulated-status', 'children'),
    [dd.Input('update-timer', 'n_intervals')])
def acquire_data(n_intervals):
    '''This function triggers the adc to take a measurement and then transfer the
    data back to us. The accumulated status is updated, triggering the update
    of both graphs.'''
    if adc.stop_loops or adc.loop_running or n_intervals == 0:
        # Prevent starting a serial connection when something is already active,
        # including another threaded instance of this function.
        raise de.PreventUpdate
    adc.loop_running = True
    logging.info("Starting acq loop")
    adc.acquisition_loop()
    adc.decode_loop()
    adc.loop_running = False
    return (f'accumulated status: {adc.accumulated_status}')


@app.callback(
    dd.Output('time-domain-graph', 'figure'),
    [dd.Input('accumulated-status', 'children'),
     dd.Input('show-windowed-button', 'n_clicks'),
     dd.Input('window-functions', 'value')])
def time_domain_update(status, show_windowed_clicks, window):
    '''Process the acquired data and display in time domain.'''
    global magnitude
    global sample_index
    try:
        magnitude = adc.decoded_data_queue[0]
        # magnitude -= np.mean(magnitude)
        # print('magnitude: {}'.format(magnitude))
        sample_index = 1 / adc.sample_rate * np.linspace(0, adc.number_of_samples,
                num=adc.number_of_samples,
                endpoint=False)
    except IndexError:
        # no data in queue, exit the function without doing anythin
        return

    show_windowed = (show_windowed_clicks % 2 != 0)

    if show_windowed:
        magnitude = (magnitude - np.mean(magnitude)) * window_map[window]

    return {'data': [{'x': sample_index, 'y': magnitude,
                      # 'type': 'line',
                      'name': 'time domain',
                      # 'mode': 'lines+markers',
                    }],
            'layout': {'title': 'Magnitude vs. sample',
                       'xaxis': {'title': 'Time [secs]',
                                 'range': [0, adc.number_of_samples/adc.sample_rate]
                                },
                       'yaxis': {'title': 'Magnitude [V]',
                                 # 'range': [-2, 2]
                                }
                      }
           }

@app.callback(dd.Output('freq-domain-graph', 'figure'),
             [dd.Input('accumulated-status', 'children'),
              dd.Input('window-functions', 'value')],
             [dd.State('freq-average-button','n_clicks'),
              dd.State('sample-rate', 'value'),
              dd.State('number-of-samples', 'value'),
              dd.State('number-of-averages', 'value')])
def freq_domain_update(status,
                       window,
                       average_clicks,
                       sample_rate,
                       number_of_samples,
                       number_of_averages):
    '''Apply a window and take the fft of the input signal to display the
    information in the frequency domain'''
    global freq_axis, freq_mag, freq_mag_history

    try:
        magnitude = adc.decoded_data_queue[0]
        freq_mag = (magnitude - np.mean(magnitude)) * window_map[window]
    except IndexError:
        # Indicates no data in the queue, exit function without doing something.
        return
    except ValueError:
        # The magnitude array is not the same length as the window. This is a
        # problem.
        logging.error(f'Input buffer size {len(magnitude)} expected {adc.number_of_samples}')
        raise de.PreventUpdate

    if number_of_averages != len(freq_mag_history):
        freq_mag_history = deque(maxlen=number_of_averages)

    # Compute the frequency magnitude in dBRMS
    freq_mag = np.abs(np.fft.rfft(a=freq_mag, n=adc.number_of_samples)
        * np.sqrt(2) / adc.number_of_samples
        / sum(window_map[window]) * len(window_map[window]))
    # Store every value and calculate the average.
    freq_mag_history.append(freq_mag)
    freq_avg_mag = np.mean(freq_mag_history, axis=0)

    average_on = (average_clicks % 2 != 0)
    if average_on:
        freq_mag_dB = 20 * np.log10(freq_avg_mag)
    else:
        freq_mag_dB = 20 * np.log10(freq_mag)

    return {'data': [{'x': freq_axis, 'y': freq_mag_dB,
                      # 'type': 'line',
                      'name': 'freq domain',
                      'mode': 'lines'}],
            'layout': {'title': 'FFT of input',
                       'xaxis': {'title': 'Frequency [Hz]',
                                 'type': 'log',
                                 # 'range': np.log10([10, adc.sample_rate/2])
                                 'range': 'auto',
                                 },
                       'yaxis': {'title': 'RMS Magnitude [dBV]',
                                 'range': [-150, 30]
                                 }
                      }
           }

if __name__ == '__main__':
    dash_log = logging.getLogger('werkzeug')
    dash_log.setLevel(logging.ERROR)
    app.run_server(debug=False)
