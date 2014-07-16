function files = getfiles(path)

files = dir(path);

for k = length(files):-1:1
    % Remove non-folders
    if files(k).isdir
        files(k) = [ ]
        continue
    end

%     % Remove folders starting with .
%     fname = files(k).name;
%     if fname(1) == '.'
%         files(k) = [ ];
%     end
end