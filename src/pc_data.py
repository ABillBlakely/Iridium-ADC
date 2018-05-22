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
from multiprocessing import Process
from threading import Thread
import numpy as np

loop_running = False

adc = SerialComms()

adc.reset()
adc.setup()
# Get and format the status register for markdown display.
status_reg_f = "\n\t" + "\n\t".join(adc.status())

magnitude = [0]
sample_index = [0]
freq_mag = [1e-12]
freq_axis = [0]
app = dash.Dash()
update_period_ms = 500

window_map = {'rect': 1,
              'blackman': np.blackman(adc.number_of_samples),
              'kaiser5': np.kaiser(adc.number_of_samples, 5 * np.pi),
              'kaiser7': np.kaiser(adc.number_of_samples, 7 * np.pi),}


app.layout = html.Div(id = 'Body',
    children=
        [
            html.H1("Iridium ADC", id='title'),
            dcc.Markdown(id='status-register', children=status_reg_f),
            html.Div(id='accumulated-status'),
            html.Button('Toggle Graph Update', id='graph-update-button'),
            html.Div(''),
            html.Label('Decimation Rate Selection:'),
            dcc.RadioItems(id='decimation-rate',
                           options=[
                                {'label':'1', 'value': '0'},
                                {'label':'2', 'value': '1'},
                                {'label':'4', 'value': '2'},
                                {'label':'8', 'value': '3'},
                                {'label':'16', 'value': '4'},
                                {'label':'32', 'value': '5'},
                                ],
                           value='5',
                          ),
            dcc.Graph(id='time-domain-graph'),
            dcc.Graph(id='freq-domain-graph'),
            html.Div([html.Label('FFT length (zero padding)'),
                      dcc.RadioItems(id='fft-length',
                                     options=[
                                         {'label': '1024', 'value': 1024},
                                         {'label': '4096', 'value': 4096},
                                         {'label': '8192', 'value': 8192},
                                         {'label': '16384', 'value': 16384},
                                         {'label': '32768', 'value': 32768},
                                         {'label': '65536', 'value': 65536},
                                         {'label': '131072', 'value': 131072},
                                         ],
                                     value=adc.number_of_samples,
                                    ),
                      html.Label('Window Function'),
                      dcc.RadioItems(id='window-functions',
                                     options=[
                                         {'label':'Rect', 'value':'rect'},
                                         {'label':'Blackman', 'value':'blackman'},
                                         {'label':'Kaiser 5', 'value':'kaiser5'},
                                         {'label':'Kaiser 7', 'value':'kaiser7'},
                                         ],
                                     value='kaiser5'
                                    ),
                     ],
                     id='fft-options-div',
                     style={'columnCount':2},),
            dcc.Interval(id='update-timer', interval=update_period_ms, n_intervals=0),
            html.Div(id='placeholder',
                     children='',
                     style={'display': 'none'}
                     ),
        ])

@app.callback(
    dd.Output('status-register', 'children'),
    [dd.Input('decimation-rate', 'value')])
def change_decimation_rate(rate):
    raise de.PreventUpdate
    # adc.change_decimation_rate(rate)
    # return "\n\t" + "\n\t".join(adc.status())


@app.callback(
    dd.Output('accumulated-status', 'children'),
    [dd.Input('update-timer', 'n_intervals')])
def update_running_status(n_intervals):
    return('accumulated status: {}'.format(adc.accumulated_status))

@app.callback(
    dd.Output('update-timer', 'interval'),
    [dd.Input('graph-update-button', 'n_clicks')])
def toggle_graph_update(n_clicks):
    if (n_clicks is None) or (n_clicks % 2 == 0):
        return update_period_ms
    else:
        # Setting disable is not working so instead just set a rediculously long
        # interval. In this case about 317 years.
        return 10e12

@app.callback(
    dd.Output('graph-update-button', 'children'),
    [dd.Input('graph-update-button', 'n_clicks')])
def update_graph_button(n_clicks):
    if (n_clicks is None) or (n_clicks % 2 == 0):
        return 'Graph updates are ON'
    else:
        return 'Graph updates are OFF'

@app.callback(
    dd.Output('placeholder', 'children'),
    [dd.Input('update-timer', 'n_intervals')])
def acquire_data(n_intervals):
    global loop_running

    if loop_running:
        return ''
    loop_running = True
    adc.acquisition_loop()
    adc.decode_loop()
    loop_running = False
    return ''

@app.callback(
    dd.Output('time-domain-graph', 'figure'),
    [dd.Input('update-timer', 'n_intervals')])
def time_domain_update(n_intervals):
    global magnitude
    global sample_index
    try:
        magnitude = adc.decoded_data_queue[0]
        magnitude -= np.mean(magnitude)
        # print('magnitude: {}'.format(magnitude))
        sample_index = 1 / adc.sample_rate * np.linspace(0, adc.number_of_samples,
                num=adc.number_of_samples,
                endpoint=False)
    except IndexError:
        pass

    return {'data': [{'x': sample_index, 'y': magnitude,
                      # 'type': 'line',
                      'name': 'time domain'}],
            'layout': {'title': 'Magnitude vs. sample',
                       'xaxis': {'title': 'Sample Index',
                                 'range': [0, adc.number_of_samples/adc.sample_rate]
                                },
                       'yaxis': {'title': 'Magnitude',
                                 'range': [-2, 2]
                                }
                      }
           }

@app.callback(dd.Output('freq-domain-graph', 'figure'),
             [dd.Input('update-timer', 'n_intervals')],
             [dd.State('fft-length', 'value'),
              dd.State('window-functions', 'value')])
def freq_domain_update(n_intervals, fft_length, window):
    global freq_axis
    global freq_mag
    try:
        magnitude = adc.decoded_data_queue[0]
        freq_mag = (magnitude - np.mean(magnitude)) * window_map[window]
    except IndexError:
        # Indicates no data in the queue, graph update happen more than data
        # acquisition so this is normal.
        pass
    except ValueError:
        # The magnitude array is not the same length as the window. this is a
        # problem.
        raise
    if fft_length != len(freq_axis):
        # Compute a new frequency axis if it does not match the old axis.

        # TODO: check sample rate as well, but this is currently not runtime
        # configurable so does not need to be done yet.
        freq_axis = np.fft.rfftfreq(n=fft_length, d=1/adc.sample_rate)
    # Compute the frequency magnitude in dB
    freq_mag = 20 * np.log10(np.abs(
        np.fft.rfft(a=freq_mag, n=fft_length) * 2 / adc.number_of_samples))

    return {'data': [{'x': freq_axis, 'y': freq_mag,
                      # 'type': 'line',
                      'name': 'freq domain'}],
            'layout': {'title': 'FFT of input',
                       'xaxis': {'title': 'Sample Index',
                                 'type': 'log',
                                 'range': np.log10([100, adc.sample_rate/2])
                                 },
                       'yaxis': {'title': 'Magnitude [dB]',
                                 'range': [-150, 10]
                                 }
                      }
           }

if __name__ == '__main__':
    log = logging.getLogger('werkzeug')
    log.setLevel(logging.CRITICAL)
    app.run_server(debug=False)
