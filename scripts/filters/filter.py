import numpy as np
from scipy.signal import kaiserord, firwin, filtfilt, group_delay
import math

def hpf_filtfilt(data, higher_cutoff, sampling_rate, transition_width,ripple_db):
    nyq_rate = sampling_rate / 2.0

    # The desired width of the transition from pass to stop,
    # relative to the Nyquist rate transition width.
    width = transition_width / nyq_rate 

    # Design a Kaiser window to limit ripple and width of transition region
    N, beta = kaiserord(ripple_db, width)

    print("Input Ripple db : {}".format(ripple_db))
    while(N%2 == 0):
        ripple_db += 1
        N, beta = kaiserord(ripple_db, width)

    print("Ripple db considered : {}".format(ripple_db))
    print("Filter order : {}".format(N))

    taps = firwin(N, higher_cutoff / nyq_rate, window=('kaiser', beta), pass_zero=False)
    # DELAY CALCULATION

    w, gd = group_delay((taps, 1.0))
    delay = math.ceil(np.mean(gd))

    filtered_signal = filtfilt(taps, 1.0, data, padlen=int(delay))

    return filtered_signal

if __name__ == "__main__":
    usage_ = "Usage : python3 {} wav_file cutoff attentuation \n\
            The cutoff must be in Hz(Anything less than 16500, gap need to ensure data is not affected\n\
            The attentuation is in dB, normal value would be 60 but anything upto 120 is fine, post that wont make a difference\n\
            Example usage : python3 {} generated.wav 16500 60".format(__file__, __file__)
    import sys

    try:
        wav_file_name = sys.argv[1]
        cutoff = float(sys.argv[2])
        ripple_db = float(sys.argv[3])
    except:
        print("Enter valid inputs")
        print(usage_)
    
    import soundfile as sf
    data,sr = sf.read(wav_file_name)

    transition_width = 350

    print("Loaded file with sample rate : {}".format(sr))

    filtered_data = hpf_filtfilt(data, cutoff, sr, transition_width, ripple_db)

    new_file_name = wav_file_name.replace(".wav", "_filtered.wav")

    sf.write(file=new_file_name, data=filtered_data, samplerate=sr)