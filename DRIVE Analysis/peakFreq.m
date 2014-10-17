function [freq] = peakFreq(data, Fs)
%PEAKFREQ Finds the frequency of the data given a sampling frequency
%
%   DATA    - The data whose frequency you want to find
%   Fs      - Sampling frequency that the data is collected at (in Hz)
%
%   FREQ    - The dominant frequency of the system (in Hz)

nPoints = length(data);
NFFT = 2^nextpow2(nPoints); 
dataF = fft (data,NFFT)/nPoints;
f = Fs/2*linspace(0,1,NFFT/2+1);
dataFmag = 2*abs(dataF(1:NFFT/2+1));
[pks, locs] = findpeaks(dataFmag);

freq = f(locs(pks == max(pks)));

end

