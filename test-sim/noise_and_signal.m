signal_level = 1e-6;
noise_level_per_rtHz = 13e-9;
sample_rate = 2e6;
sequence_length = 256 * 1024/4;  % 256 kB worth of samples
% signal_freq = sample_rate / sequence_length * 100;
signal_freq = 1000;
bit_depth = 24;

    %  test setup:
    % making an assumption that the samples are not stored across words,
    % so a 24 bit sample takes up 4 bytes (one 32 bit word).
    % 1024 bytes per kilobyte / 4 bytes per sample = 256 samples per kilobyte
    % 1024 kilobytes per megabyte * 256 samples per kilobyte = 262144 samples per megabyte

    % [mag, index] = noise_and_signal(1000, 1e-3, 1e-6, 1e6, 256, 24)


    % From http://mdc.custhelp.com/app/answers/detail/a_id/16847/~/relationship-between-rms-noise-and-peak-to-peak-noise
    % > When it comes to noise, we are doing some approximations.
    % > We are assuming white (Gaussian) noise but technically,
    % > random white noise will have some rare spikes which reach
    % > towards infinity. We ignore these. The instantaneous
    % > peak-to-peak amplitude of white noise will be less than 8x
    % > the RMS value around 98% of the time. And in practice,
    % > when you view noise on the scope, the trained eye
    % > seems to find this 98% level pretty consistently. So we
    % > have intertwined noise theory and practicality to come
    % > up with our 8x figure.
    %
    % This does not seem to hold true when using the matlab randn function to generate noise.

    % digital index array:
    sequence = [0:sequence_length - 1];

    % "Perfect" analog signal:
    signal_float = signal_level * sin(2*pi*signal_freq * sequence / sample_rate);
    % simulate quantization
    quantized_signal = round(signal_float * 2^(bit_depth-1)) / 2^(bit_depth-1);
    % calculate the RMS quantization error just to check. Should be approx. 6.02*N + 1.76
    quantized_error_dB = 20*log10(sqrt(sum((quantized_signal - signal_float).^2/sequence_length)))

    % Caluclate the rms Noise level from the Noise density.
    noise_rms = noise_level_per_rtHz * sqrt(sample_rate/2);
    % Use normally distributed random numbers to simulate the white noise.
    white_noise_float = randn([1, sequence_length]) * noise_rms;
    % Calculate the actual rms value of the created signal to cross check with Spice.
    calc_noise_rms = sqrt(sum(white_noise_float.^2)/sequence_length)

    % Sum the signals and quantize.
    total_signal_quantized = round((white_noise_float + signal_float) * 2^(bit_depth-1)) / 2^(bit_depth-1);

    % Choose a window:
    % window = chebwin(sequence_length, 120)';
    window = kaiser(sequence_length, 7*pi)';
    % window = flattopwin(sequence_length)';
    % window = rectwin(sequence_length)';

    % Perform fft with magnitude scaling.
    signal_mag = abs(fft(signal_float .* window) * 2 / sequence_length);
    quantized_signal_mag = abs(fft(quantized_signal .* window) * 2 / sequence_length);
    quantized_plus_noise_mag = abs(fft(total_signal_quantized .* window) * 2 / sequence_length);
    freq_axis = sequence * sample_rate / sequence_length;

    figure(1)
    subplot(211);
    semilogx(freq_axis, 20*log10(signal_mag), 'r:', 'LineWidth', 2);
    hold on;
    xlim([10, 1e6]);
    ylim([-200,0]);
    xlabel('Frequency [Hz]');
    ylabel('Magnitude [dB]');
    % waitforbuttonpress;
    semilogx(freq_axis, 20*log10(quantized_signal_mag), 'k');
    % waitforbuttonpress;
    semilogx(freq_axis, 20*log10(quantized_plus_noise_mag), 'b', 'LineWidth', 2);
    hold off;
    legend(['Internal floating point precision'], ...
            sprintf('Quantized to %d bits', bit_depth), ...
            ['Signal plus noise and quantization.'], ...
            'Location', 'northwest')
    title('Simulation of 1 MB data block with noise and quantization, FFT analysis')

    subplot(212)
    plot(sequence(1:round(sample_rate/signal_freq) * 4)/sequence_length, total_signal_quantized(1:round(sample_rate/signal_freq) * 4));
    title('Four periods of input signal + noise')
    xlabel('Time [secs]')
    ylabel('Signal plus noise [V]')
