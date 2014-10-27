function [ fileArrayNew ] = minFileSize( directory, fileArray, fileSize )
%MINFILESIZE Finds files within a directory that are above a certain size
%
%   DIRECTORY   - the directory containing the fileArray
%   FILEARRAY   - the list of the names of the files whose size to check
%   FILESIZE    - threshold size (in MB) to look for
%
%   FILEARRAYNEW - the list of file name larger than the threshold size

fileCheckArray = zeros(1,length(fileArray));

for file = 1:length(fileArray)
    fileInfo = dir(strcat(directory, fileArray(file).name));
    if fileInfo.bytes > fileSize*1000000;
        fileCheckArray(file) = true;
    else 
        fileCheckArray(file) = false;
    end
end

% Remove all files below the threshold
fileCheckArray = logical(fileCheckArray);
fileArrayNew = fileArray(fileCheckArray);

end

