function [ bioimp, ultrasound ] = getDRIVEdata
%UNTITLED3 Summary of this function goes here
%   Detailed explanation goes here

% Select patient folder
dirPatient = uigetdir('','Select the patient directory');
dirUltrasound = strcat(dirPatient,'/ultrasound/');

addpath(genpath(dirPatient));
cd(dirPatient);
patientID = dirPatient((end-7):end);
disp(horzcat('Working on subject: ',patientID))

% Select the ultrasound files
ultrasoundFileArrayInfo = dir(dirUltrasound);
fileArrayUltrasound = ultrasoundFileArrayInfo(arrayfun(@(x) x.name(1), ...
    ultrasoundFileArrayInfo) ~= '.');

fileUltrasound = minFileSize(dirUltrasound,fileArrayUltrasound, 35);

% Select the bioimpedance files
bioimpedanceFileArrayInfo = dir(dirPatient);
bioimpedanceFileArray = bioimpedanceFileArrayInfo(arrayfun(@(x) x.name(1), ...
    bioimpedanceFileArrayInfo) ~= '.');
bioimpedanceFileArray = bioimpedanceFileArray(arrayfun(@(x) x.isdir, ...
    bioimpedanceFileArray) ~= 1);

for indFile = 1:length(bioimpedanceFileArray)
    fileBioimp{indFile} = bioimpedanceFileArray(indFile).name;
end

% Put everything into structs
bioimp.dir = dirPatient;
ultrasound.dir = dirUltrasound;
bioimp.files = fileBioimp;
ultrasound.files = fileUltrasound;


end

