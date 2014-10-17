function [freq] = peakFreq(data, Fs, type, Fc)
%PEAKFREQ Finds the frequency of the data given a sampling frequency
%
%   DATA    - The data whose frequency you want to find
%   Fs      - Sampling frequency that the data is collected at (in Hz)
%   type    - Which way the user would like to look for peaks
%   Fc      - The cutoff frequency the user wants to look around
%
%   FREQ    - The dominant frequency of the system (in Hz)

if nargin < 2
    fprintf('Error: not enough input arguments.\n')
    fprintf('Please provide both the data and the sampling frequency.\n')
end

nPoints = length(data);
NFFT = 2^nextpow2(nPoints);
dataF = fft (data,NFFT)/nPoints;
f = Fs/2*linspace(0,1,NFFT/2+1);
dataFmag = 2*abs(dataF(1:NFFT/2+1));

switch nargin
    case 2
        [pks, locs] = findpeaks(dataFmag);
        freq = f(locs(pks == max(pks)));
    case 3
        fprintf('Error: when using type function, give cutoff frequency.\n')
    case 4
        switch type
            case 'low'
                fLow = f(f < Fc);
                [pks, locs] = findpeaks(dataFmag(f < Fc));
                freq = fLow(locs(pks == max(pks)));
            case 'high'
                fHigh = f(f > Fc);
                [pks, locs] = findpeaks(dataFmag(f > Fc));
                freq = fHigh(locs(pks == max(pks)));
            case 'band'
                if length(Fc) == 2
                fBand = f(f > Fc(1) & f < Fc(2));
                [pks, locs] = findpeaks(dataFmag(f > Fc(1) & f < Fc(2)));
                freq = fBand(locs(pks == max(pks)));
                else
                    fprintf('Cutoff Frequency should be of the form:\n')
                    fprintf('[freqLow freqHigh]\n')
                end
        end
end
end
