function [ data ] = getRespAndCard( bioimpData, Fs )
%GETRESPANDCARD Summary of this function goes here
%   Detailed explanation goes here


data.smoothed = smooth(bioimpData,Fs/2);
data.smoothed = smooth(data.smoothed, Fs/10);
data.resp = smooth(data.smoothed, Fs);
data.card = smooth(data.smoothed - data.resp);

end

