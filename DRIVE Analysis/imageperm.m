function I_new = imageperm(I)
% IMAGEPERM     Changes the shapes of 4D DICOM data

if size(I,3) == 3;
    I_new = permute(I, [1 2 4 3]);
elseif size(I,4) == 1;
    error('imageperm:size', 'This data does not contain multiple frames')
end