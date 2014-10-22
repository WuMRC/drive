function [ fileArrayNew ] = minFileSize( directory, fileArray, fileSize )
%UNTITLED7 Summary of this function goes here
%   Detailed explanation goes here

fileCheckArray = zeros(1,length(fileArray));

for file = 1:length(fileArray)
    fileInfo = dir(strcat(directory, fileArray(file).name));
    if fileInfo.bytes > fileSize
        fileCheckArray(file) = true;
    else 
        fileCheckArray(file) = false;
    end
end

% Remove all nonrelavent ultrasound images
fileCheckArray = logical(fileCheckArray);
fileArrayNew = fileArray(fileCheckArray);

end

