function [ACQmarkersInd] = getACQmarkers(path, file)
%UNTITLED5 Summary of this function goes here
%   Detailed explanation goes here

acq = load_acq(strcat(path,file));
if isfield(acq.markers,'lMarkers') == 1
    timeStartACQ = acq.hdr.graph.first_time_offset/1000;
    
    day = 60*60*24;
    timeStartACQ = str2double(datestr(timeStartACQ/day,'HHMMSS'));
    
    shift = 0;
    % Find markers in ACQKnowledge data
    ACQmarkers = zeros(1,length(acq.markers.lSample));
    ACQmarkersInd = zeros(1,length(acq.markers.lSample));
    for nMarkers = 1:length(acq.markers.lSample)
        if acq.markers.lSample(nMarkers) == 0
            shift = shift + 1;
        else
            x = str2double(datestr(timeStartACQ/day+...
                (double(acq.markers.lSample(nMarkers)/200)/day),'HHMMSS'));
            ACQmarkers(nMarkers-shift) = x;
            ACQmarkersInd(nMarkers-shift) = acq.markers.lSample(nMarkers);
        end
    end
    clear x
    
    ACQmarkersInd = unique(ACQmarkersInd);
    
else
    fprint('There are no markers to display')
    ACQmarkers = NaN;
    
end

