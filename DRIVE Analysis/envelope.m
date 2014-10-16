function [dataEnv] = envelope(dataX, dataY, side, nPoints) 
%ENVELOPE   Gets the envelope of a signal from a chosen side (top or bottom)
%
%   [dataEnv] = envelope(dataX, dataY, side, nPoints) returns the top or
%   bottom envelope of a chosen signal as a vector of the same length by
%   finding local maxima around a number of points and interpolating
%   between them over the original length of the x-data
%
%   dataEnv - the enveloped data
%   dataX - the x-data
%   dataY - the y-data
%   side - the chosen side (either 'top' or 'bottom')
%   nPoints - the size of the region of interest to observe maxima/minima
%
% Original author: Andreas Martin, Volkswagen AG, Germany
% Modified by Barry Belmont, WuMRC, USA

side = strcmpi({'top','bottom'}, side) * [1; -1];

dataY = dataY(:);
dataLength = length(dataY);
xNew = [];
yNew = [];

ind = 1;
while ind < dataLength;
    ii = ind+1:min(ind + nPoints, dataLength);
    [ m, idx ] = max((dataY(ii) - dataY(ind)) ./ (ii-ind)' .* side);

    % Equidistant x_data assumed! Use next row instead, if not:
    %[ m, idx ] = max( ( y_data(ii) - y_data(i) ) ./ ( x_data(ii) - x_data(i) ) * side );
    
    % New max. slope: store new "observation point"
    ind = ind + idx;
    xNew = [xNew dataX(ind)];
    yNew = [yNew dataY(ind)];
end;

dataEnv = interp1( xNew, yNew, dataX, 'linear', 'extrap');
