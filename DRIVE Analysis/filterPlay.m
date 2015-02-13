

t = 1:0.01:100;
x = sin(t)+0.5*sin(2.3*t)+0.1*sin(1.2*t)-0.7*cos(t*0.3);

plot(t,x)

%%

dt = 1/3;
tau = 1/(2*pi*1)
a = dt/tau;

y = filter(a,[1 a-1],x);

plot(t,y)

%%

Fs = 200;                    % Sampling frequency
T = 1/Fs;                     % Sample time
L = length(bioimp.nb.data(:,1));                     % Length of signal
t = (0:L-1)*T;                % Time vector
% Sum of a 50 Hz sinusoid and a 120 Hz sinusoid
% x = 0.7*sin(2*pi*50*t) + sin(2*pi*120*t); 
y = bioimp.nb.data(:,1);
plot(t,bioimp.nb.data(:,1))

NFFT = 2^nextpow2(L); % Next power of 2 from length of y
Y = fft(y,NFFT)/L;
f = Fs/2*linspace(0,1,NFFT/2+1);

% Plot single-sided amplitude spectrum.
plot(f,2*abs(Y(1:NFFT/2+1))) 
title('Single-Sided Amplitude Spectrum of y(t)')
xlabel('Frequency (Hz)')
ylabel('|Y(f)|')


%%

f=10; %frequency of sine wave
overSampRate=30; %oversampling rate
fs=overSampRate*f; %sampling frequency
phase = 1/3*pi; %desired phase shift in radians
nCyl = 5; %to generate five cycles of sine wave
 
t=0:1/fs:nCyl*1/f; %time base
 
x=sin(2*pi*f*t+phase); %replace with cos if a cosine wave is desired
plot(t,x);
title(['Sine Wave f=', num2str(f), 'Hz']);
xlabel('Time(s)');
ylabel('Amplitude');

%%

f=10; %frequency of sine wave
overSampRate=30; %oversampling rate
fs=overSampRate*f; %sampling frequency
phase = 1/3*pi; %desired phase shift in radians
nCyl = 5; %to generate five cycles of sine wave
 
t=0:1/fs:nCyl*1/f; %time base

fs = 200;                    % Sampling frequency
T = 1/Fs;                     % Sample time
L = length(bioimp.nb.data(:,1));                     % Length of signal
t = (0:L-1)*T; 

x=bioimp.nb.data(:,1)-mean(bioimp.nb.data(:,1)); %replace with cos if a cosine wave is desired
plot(t,x);
title(['Sine Wave f=', num2str(f), 'Hz']);
xlabel('Time(s)');
ylabel('Amplitude');
%%

NFFT=1024; %NFFT-point DFT          
X=fft(x,NFFT); %compute DFT using FFT          
nVals=0:NFFT-1; %DFT Sample points          
plot(nVals,abs(X));          
title('Double Sided FFT - without FFTShift');          
xlabel('Sample points (N-point DFT)')          
ylabel('DFT Values');


NFFT=1024; %NFFT-point DFT          
X=fft(x,NFFT); %compute DFT using FFT          
nVals=(0:NFFT-1)/NFFT; %Normalized DFT Sample points          
plot(nVals,abs(X));          
title('Double Sided FFT - without FFTShift');          
xlabel('Normalized Frequency')          
ylabel('DFT Values');


NFFT=1024; %NFFT-point DFT          
X=fft(x,NFFT); %compute DFT using FFT          
fVals=(-NFFT/2:NFFT/2-1)/NFFT; %DFT Sample points          
plot(fVals,abs(X));          
title('Double Sided FFT - with FFTShift');          
xlabel('Normalized Frequency')          
ylabel('DFT Values');

NFFT=1024;          
X=fftshift(fft(x,NFFT));          
fVals=fs*(-NFFT/2:NFFT/2-1)/NFFT;          
plot(fVals,abs(X),'b');          
title('Double Sided FFT - with FFTShift');          
xlabel('Frequency (Hz)')          
ylabel('|DFT Values|');

NFFT=1024;
L=length(x);          
X=fftshift(fft(x,NFFT));          
Px=X.*conj(X)/(NFFT*L); %Power of each freq components          
fVals=fs*(-NFFT/2:NFFT/2-1)/NFFT;          
plot(fVals,Px,'b');          
title('Power Spectral Density');          
xlabel('Frequency (Hz)')          
ylabel('Power');

%%
x = bioimp.nb.data(:,1)-mean(bioimp.nb.data(:,1));
dataL = length(x);
filtL = length(x)+1;
fs = 200;
fc = 2;

hsupp = (-(filtL-1)/2:(filtL-1)/2);
hideal = (2*fc/fs)*si   nc(2*fc*hsupp/fs);

h = hamming(filtL)' .* hideal; % h is our filter

% Choose the next power of 2 greater than L+M-1 
Nfft = 2^nextpow2(dataL+filtL-1); % 2^(ceil(log2(filtL+dataL-1))); %

% Zero pad the signal and impulse response:
xzp = [ x' zeros(1,Nfft-dataL) ];
hzp = [ h zeros(1,Nfft-filtL) ];

X = fft(xzp); % signal
H = fft(hzp); % filter

Y = X .* H;

y = ifft(Y);
relrmserr = norm(imag(y))/norm(y) % check... should be zero
y = real(y);

plot(y)