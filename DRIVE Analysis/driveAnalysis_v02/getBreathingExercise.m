function [ exerciseID ] = getBreathingExercise( bioimp )
%UNTITLED2 Summary of this function goes here
%   Detailed explanation goes here

% Initialize the array
exerciseArray = zeros(bioimp.acq.markers.lMarkers,1);

for indExercise = 1:bioimp.acq.markers.lMarkers
    
    if findstr(bioimp.acq.markers.szText{1,indExercise},'nb') > 0
        exerciseArray(indExercise) = 1;
    elseif findstr(bioimp.acq.markers.szText{1,indExercise},'held') > 0
        exerciseArray(indExercise) = 2;
    elseif findstr(bioimp.acq.markers.szText{1,indExercise},'pep') > 0
        exerciseArray(indExercise) = 3;
    elseif findstr(bioimp.acq.markers.szText{1,indExercise},'imt') > 0
        exerciseArray(indExercise) = 4;
    else
        exerciseArray(indExercise) = 0;
    end

end

exerciseID = exerciseArray;



end

