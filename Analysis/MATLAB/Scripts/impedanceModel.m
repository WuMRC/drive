
Ri = 972; % Internal cellular resistance
Re = 667; % External cellular resistance
C = 4.7 * 10^-9; % Cell membrane capacitance
R0 = Re; % Resistance with DC Voltage source
Rinf = (Re * Ri) / (Re + Ri); % Resistance at infinite alternating frequency
Rdelta = R0 - Rinf; % Difference between R0 and Rinf
tau = (Re+Ri)*C; % Time constant of model circuit
k = 2 * pi *1000; % Factors of omega in w = 2*pi*f

% Resistance and reactance model functions (Frequency in Kilohertz)

rm = Rinf + (Rdelta ./ (1 + (k^2 * tau^2 *  f.^2)));
xm = Rdelta * k * tau * f ./ (1 + (k^2 * tau^2 * f.^2));

