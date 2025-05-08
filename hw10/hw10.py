import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import numpy.fft as fft

def plot_fft(signal, nyquist, title):

    # time
    Ts = 1.0/nyquist
    n = len(signal)
    t = np.arange(0, n) * Ts
    Y = fft.fft(signal) / n

    # frequency

    T = n/nyquist
    k = np.arange(n)
    frq = k/T

    # one side frequency range
    half_n = int(n/2)
    frq = frq[:half_n]
    Y = Y[:half_n]

    fig, (ax1, ax2) = plt.subplots(2, 1)
    ax1.plot(t,signal,'b')
    ax1.set_xlabel('Time')
    ax1.set_ylabel('Amplitude')
    ax2.loglog(frq,abs(Y),'b')
    ax2.set_xlabel('Freq (Hz)')
    ax2.set_ylabel('|Y(freq)|')
    fig.suptitle(title) 
    plt.tight_layout() 
    plt.savefig(f'FFT/ {title}_fft.png')
    plt.close()

    return Y, frq

def plot_original_signals(df_A, df_B, df_C, df_D):
    plt.subplot(2, 2, 1)
    plt.plot(df_A['time'], df_A['sigA'])
    plt.title('Signal A')
    plt.subplot(2, 2, 2)
    plt.plot(df_B['time'], df_B['sigB'])
    plt.title('Signal B')
    plt.subplot(2, 2, 3)
    plt.plot(df_C['time'], df_C['sigC'])
    plt.title('Signal C')
    plt.subplot(2, 2, 4)
    plt.plot(df_D['time'], df_D['sigD'])
    plt.title('Signal D')
    plt.suptitle('Original Signals')
    plt.tight_layout()
    plt.savefig('original_signals.png')
    plt.close()



def plot_compare_fft(signals_data):
    plt.figure()
    plt.subplot(2, 2, 1)
    for i, signal in enumerate(signals_data):
        original_fft, filtered_data, title, original_frq, nyquist = signal

        # Calculate FFT of filtered data
        n = len(filtered_data)
        T = n/nyquist
        k = np.arange(n)
        frq = k/T
        Y = fft.fft(filtered_data) / n

        # One side frequency range
        half_n = int(n/2)
        frq = frq[:half_n]
        Y = Y[:half_n]

        plt.subplot(2, 2, i+1)
        plt.loglog(original_frq, abs(original_fft), 'black', label='Original')
        plt.loglog(frq,abs(Y),'red', label='Filtered')
        plt.xlabel('Frequency (Hz)')
        plt.ylabel('|Y(freq)|')
        plt.title(title) 
        plt.tight_layout()
 

    plt.subplots_adjust(top=0.85, hspace=0.8)
    plt.suptitle('Compare FFT with Coefficients')
    plt.savefig(f'Compare/compare_coefficients.png')
    plt.close()


def MAF(signal, time, window_size, title):
    filter_data = []
    for i in range(0, len(signal)-window_size + 1):
        window = signal[i:i+window_size]
        filter_data.append(np.mean(window))

    plt.plot(time.iloc[0:len(filter_data)], filter_data)
    plt.title(title)
    plt.savefig(f'MAF/{title}_MAF.png')
    plt.close()

    return filter_data

def IIR(signal, time, title, alpha, beta):
    filter_data = np.zeros(len(signal))
    filter_data[0] = alpha * signal[0]  # Initial condition
    
    # Apply IIR filter
    for i in range(1, len(signal)):
        filter_data[i] = beta * signal[i] + alpha * filter_data[i-1]
    
    plt.figure()
    plt.plot(time, filter_data)
    plt.title(f'IIR Filter: {title}')
    plt.xlabel('Time')
    plt.ylabel('Amplitude')
    plt.savefig(f'IIR/{title}_IIR.png')
    plt.close()
    
    return filter_data


def filter_with_coefficients(signal, time, title, coefficients):
    # Apply the filter using convolution
    filtered_data = np.convolve(signal, coefficients, mode='valid')

    
    return filtered_data



if __name__ == "__main__":

    df_A = pd.read_csv('sigA.csv')
    df_B = pd.read_csv('sigB.csv')
    df_C = pd.read_csv('sigC.csv')
    df_D = pd.read_csv('sigD.csv')

    df_A.columns = ['time', 'sigA']
    df_B.columns = ['time', 'sigB']
    df_C.columns = ['time', 'sigC']
    df_D.columns = ['time', 'sigD']

    num_samples_A = len(df_A)
    num_samples_B = len(df_B)
    num_samples_C = len(df_C)
    num_samples_D = len(df_D)

    total_time_A = df_A['time'].iloc[-1]
    total_time_B = df_B['time'].iloc[-1]
    total_time_C = df_C['time'].iloc[-1]
    total_time_D = df_D['time'].iloc[-1]

    nyquist_A = num_samples_A / total_time_A
    nyquist_B = num_samples_B / total_time_B
    nyquist_C = num_samples_C / total_time_C
    nyquist_D = num_samples_D / total_time_D

    fft_A = fft.fft(df_A['sigA'])

    print(f"Nyquist frequency of Signal A: {nyquist_A} Hz")
    print(f"Nyquist frequency of Signal B: {nyquist_B} Hz")
    print(f"Nyquist frequency of Signal C: {nyquist_C} Hz")
    print(f"Nyquist frequency of Signal D: {nyquist_D} Hz")

    original_fft_A, original_frq_A = plot_fft(df_A['sigA'], nyquist_A, 'Signal A')
    original_fft_B, original_frq_B = plot_fft(df_B['sigB'], nyquist_B, 'Signal B')
    original_fft_C, original_frq_C = plot_fft(df_C['sigC'], nyquist_C, 'Signal C')
    original_fft_D, original_frq_D = plot_fft(df_D['sigD'], nyquist_D, 'Signal D')


    filtered_A = MAF(df_A['sigA'], df_A['time'], 100, 'Signal A')
    filtered_B = MAF(df_B['sigB'], df_B['time'], 100, 'Signal B')
    filtered_C = MAF(df_C['sigC'], df_C['time'], 100, 'Signal C')
    filtered_D = MAF(df_D['sigD'], df_D['time'], 100, 'Signal D')

    signals_data_MAF = [
        (original_fft_A, filtered_A, 'Signal_A', original_frq_A, nyquist_A),
        (original_fft_B, filtered_B, 'Signal_B', original_frq_B, nyquist_B),
        (original_fft_C, filtered_C, 'Signal_C', original_frq_C, nyquist_C),
        (original_fft_D, filtered_D, 'Signal_D', original_frq_D, nyquist_D)
    ]

    #plot_compare_fft(signals_data_MAF)

    alpha = 0.9
    beta = 0.1
    filtered_A_IIR = IIR(df_A['sigA'], df_A['time'], 'Signal A', alpha, beta)
    filtered_B_IIR = IIR(df_B['sigB'], df_B['time'], 'Signal B', alpha, beta)
    filtered_C_IIR = IIR(df_C['sigC'], df_C['time'], 'Signal C', alpha, beta)
    filtered_D_IIR = IIR(df_D['sigD'], df_D['time'], 'Signal D', alpha, beta)

    signals_data_IIR = [
        (original_fft_A, filtered_A_IIR, 'Signal_A', original_frq_A, nyquist_A),
        (original_fft_B, filtered_B_IIR, 'Signal_B', original_frq_B, nyquist_B),
        (original_fft_C, filtered_C_IIR, 'Signal_C', original_frq_C, nyquist_C),
        (original_fft_D, filtered_D_IIR, 'Signal_D', original_frq_D, nyquist_D)
    ]

    #plot_compare_fft(signals_data_IIR)

    coefficients = [
        0.000000000000000000,
        0.000000000000000000,
        0.000007188274115481,
        0.000026999392415735,
        0.000049699512645601,
        0.000049863203532925,
        0.000000000000000000,
        -0.000106466630123868,
        -0.000235784361628246,
        -0.000313423411736154,
        -0.000251660491775780,
        0.000000000000000000,
        0.000403178328101412,
        0.000808959582588773,
        0.000992137799152341,
        0.000744815418518378,
        -0.000000000000000001,
        -0.001072273934694242,
        -0.002059763470923100,
        -0.002430325033226228,
        -0.001762349297820786,
        0.000000000000000002,
        0.002390452110110751,
        0.004475318846985163,
        0.005158538261110197,
        0.003662355995274633,
        -0.000000000000000003,
        -0.004791312805438043,
        -0.008836419633830974,
        -0.010054535287361244,
        -0.007062019894287973,
        0.000000000000000005,
        0.009108666787451508,
        0.016750199379556279,
        0.019068143596055322,
        0.013452460567012516,
        -0.000000000000000006,
        -0.017780693242571512,
        -0.033453572438736034,
        -0.039359812124647071,
        -0.029101364265679112,
        0.000000000000000007,
        0.045364163923913244,
        0.099190334251021056,
        0.150217079433425715,
        0.186747434189619838,
        0.200007574943747768,
        0.186747434189619838,
        0.150217079433425743,
        0.099190334251021070,
        0.045364163923913244,
        0.000000000000000007,
        -0.029101364265679108,
        -0.039359812124647078,
        -0.033453572438736048,
        -0.017780693242571508,
        -0.000000000000000006,
        0.013452460567012516,
        0.019068143596055322,
        0.016750199379556286,
        0.009108666787451508,
        0.000000000000000005,
        -0.007062019894287979,
        -0.010054535287361239,
        -0.008836419633830977,
        -0.004791312805438043,
        -0.000000000000000003,
        0.003662355995274636,
        0.005158538261110193,
        0.004475318846985165,
        0.002390452110110752,
        0.000000000000000002,
        -0.001762349297820787,
        -0.002430325033226230,
        -0.002059763470923101,
        -0.001072273934694242,
        -0.000000000000000001,
        0.000744815418518378,
        0.000992137799152341,
        0.000808959582588773,
        0.000403178328101411,
        0.000000000000000000,
        -0.000251660491775781,
        -0.000313423411736154,
        -0.000235784361628246,
        -0.000106466630123867,
        0.000000000000000000,
        0.000049863203532925,
        0.000049699512645602,
        0.000026999392415735,
        0.000007188274115481,
        0.000000000000000000,
        0.000000000000000000,
    ]

    filtered_A_FIR = filter_with_coefficients(df_A['sigA'], df_A['time'], 'Signal A', coefficients)
    filtered_B_FIR = filter_with_coefficients(df_B['sigB'], df_B['time'], 'Signal B', coefficients)
    filtered_C_FIR = filter_with_coefficients(df_C['sigC'], df_C['time'], 'Signal C', coefficients)
    filtered_D_FIR = filter_with_coefficients(df_D['sigD'], df_D['time'], 'Signal D', coefficients)

    # Create data for comparison plots
    signals_data_FIR = [
        (original_fft_A, filtered_A_FIR, 'Signal_A', original_frq_A, nyquist_A),
        (original_fft_B, filtered_B_FIR, 'Signal_B', original_frq_B, nyquist_B),
        (original_fft_C, filtered_C_FIR, 'Signal_C', original_frq_C, nyquist_C),
        (original_fft_D, filtered_D_FIR, 'Signal_D', original_frq_D, nyquist_D)
    ]

    # Plot comparison
    plot_compare_fft(signals_data_FIR)






