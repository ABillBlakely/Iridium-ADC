import serial
import io
import dash
import dash_core_components as dcc
import dash_html_components as html
import dash.dependencies as dd
from adc_comms import SerialComms

# deques are used for thread safe sharing of data
from collections import deque
from multiprocessing import Process
from threading import Thread
import numpy as np

adc_comms = SerialComms()

# Get and format the status register for markdown display.
status_reg_f = "\n\t" + "\n\t".join(adc_comms.status())

# TODO: verify the status register before starting sampling.
# put the adc into sampling mode.
adc_comms.start_sampling()

adc_comms.acquisition_loop()
adc_comms.acquisition_loop()

magnitude = [0]
sample_index = [0]
freq_mag = [0]
freq_axis = [0]
app = dash.Dash()

app.layout = html.Div(id = 'Body',
    children=
        [
            html.H1("Iridium ADC", id='title'),
            dcc.Markdown(status_reg_f, id='text-box'),
            html.Div(id='accumulated-status'),
            html.Button('Toggle Graph Update', id='graph-update-button'),
            dcc.Graph(id='time-domain-graph'),
            dcc.Graph(id='freq-domain-graph'),
            dcc.Interval(id='update-timer', interval=500, n_intervals=0),
        ])

@app.callback(
    dd.Output('accumulated-status', 'children'),
    [dd.Input('update-timer', 'n_intervals')])
def update_running_status(n_intervals):
    return('accumulated status: {}'.format(adc_comms.accumulated_status))

@app.callback(
    dd.Output('update-timer', 'interval'),
    [dd.Input('graph-update-button', 'n_clicks')])
def toggle_graph_update(n_clicks):
    print(n_clicks)
    if (n_clicks is None) or (n_clicks % 2 == 0):
        # Setting disable is not working so instead just set a rediculously long
        # interval. In this case about 11 days.
        return 10e8
    else:
        return 200

@app.callback(
    dd.Output('graph-update-button', 'children'),
    [dd.Input('graph-update-button', 'n_clicks')])
def toggle_graph_update(n_clicks):
    if (n_clicks is None) or (n_clicks % 2 == 0):
        # Setting disable is not working so instead just set a rediculously long
        # interval. In this case about 11 days.
        return 'Graph updates are OFF'
    else:
        return 'Graph updates are ON'

@app.callback(
    dd.Output('title', 'children'),
    [dd.Input('update-timer', 'n_intervals')])
def time_domain_update(n_intervals):
    adc_comms.acquisition_loop()
    return 'Iridium ADC'

@app.callback(
    dd.Output('time-domain-graph', 'figure'),
    [dd.Input('update-timer', 'n_intervals')])
def time_domain_update(n_intervals):
    global magnitude
    global sample_index
    try:
        adc_comms.decode_loop()
        magnitude = adc_comms.decoded_data_queue[0]
        # print('magnitude: {}'.format(magnitude))
        sample_index = [nn for nn in range(len(magnitude))]
        fft_mag = 20 * np.log10(np.abs(
            np.fft.rfft(a=magnitude, n=2**13) * 2 / 1024))
    except IndexError: pass
    except: raise

    return {'data': [{'x': sample_index, 'y': magnitude,
                      # 'type': 'line',
                      'name': 'time domain'}],
            'layout': {'title': 'Magnitude vs. sample',
                       'xaxis': {'title': 'Sample Index',
                                 'range': [0, 1024]
                                 },
                       'yaxis': {'title': 'Magnitude',
                                 'range': [-0.5 * 4.096, 0.5 * 4.096]
                                 }
                      }
           }

@app.callback(
    dd.Output('freq-domain-graph', 'figure'),
    [dd.Input('update-timer', 'n_intervals')])
def time_domain_update(n_intervals):
    global freq_mag
    global freq_axis
    try:
        magnitude = adc_comms.decoded_data_queue[0] * np.kaiser(1024, 7 * np.pi)
    except IndexError: pass

    freq_axis = [nn for nn in range(2**13)]
    freq_mag = 20 * np.log10(np.abs(
        np.fft.rfft(a=magnitude, n=2**13) * 2 / 1024))

    return {'data': [{'x': freq_axis, 'y': freq_mag,
                      # 'type': 'line',
                      'name': 'freq domain'}],
            'layout': {'title': 'FFT of input',
                       'xaxis': {'title': 'Sample Index',
                                 # 'range': [0, 2*13]
                                 },
                       'yaxis': {'title': 'Magnitude [dB]',
                                 'range': [-150, 10]
                                 }
                      }
           }

if __name__ == '__main__':
    app.run_server(debug=False)
