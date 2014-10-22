function [totalData] = getImpedance(path, file)
%UNTITLED3 Summary of this function goes here
%   Detailed explanation goes here

acq = load_acq(strcat(path,file));

% cd(bioimpedancePath);

ohmsPerVolt = 20;

bArm = acq.data(:,1)*ohmsPerVolt;
bLeg = acq.data(:,2)*ohmsPerVolt;
ppg = acq.data(:,3); 

dt = acq.hdr.graph.sample_time*0.001;
Fs = 1/dt;

bLegPreSMOOTH = smooth(bLeg,Fs/2);
bLegSMOOTH = smooth(bLegPreSMOOTH, Fs/10);
respLegSMOOTH = smooth(bLegSMOOTH, Fs);
cardLegSMOOTH = smooth(bLegSMOOTH - respLegSMOOTH);

bArmPreSMOOTH = smooth(bArm,Fs/2);
bArmSMOOTH = smooth(bArmPreSMOOTH, Fs/10);
respArmSMOOTH = smooth(bArmSMOOTH, Fs);
cardArmSMOOTH = smooth(bArmSMOOTH - respArmSMOOTH);

totalData = [bLegSMOOTH, respLegSMOOTH, cardLegSMOOTH, ...
    bArmSMOOTH, respArmSMOOTH, cardArmSMOOTH, ppg];

end

