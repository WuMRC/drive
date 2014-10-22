function [ctotal, rowMove, colMove] = microblockanalysis(imageStack,blockSize,frame2frame)

[nBlockRows, nBlockCols, nFrames] = size(imageStack);

% Check to see if the user passed an even block size and change it to one
% slightly larger if need be
if mod(blockSize,2) == 0
    blockSize = blockSize + 1;
end
    
total = nFrames-frame2frame;
for indFrames = 1:total
    
    % Get frame to frame data
    currentFrameData = imageStack(:,:,indFrames);
    nextFrameData = imageStack(:,:,indFrames+frame2frame);
    
    
    for indRow = 1:blockSize:nBlockRows-blockSize
        for indCol = 1:blockSize:nBlockCols-blockSize
            
            indRowOffset = indRow + ceil(blockSize/2);
            indColOffset = indCol + ceil(blockSize/2);
            
            currentBlock = currentFrameData(...
                ceil(indRowOffset-blockSize/2):ceil(indRowOffset+blockSize/2),...
                ceil(indColOffset-blockSize/2):ceil(indColOffset+blockSize/2));
            nextBlock = nextFrameData(...
                ceil(indRowOffset-blockSize/2):ceil(indRowOffset+blockSize/2),...
                ceil(indColOffset-blockSize/2):ceil(indColOffset+blockSize/2));
            
            indRowSave = (indRow+blockSize-1)/blockSize;
            indColSave = (indCol+blockSize-1)/blockSize;
            x = normxcorr2(currentBlock,nextBlock);
            y = x(ceil(blockSize/2+1):ceil(2*blockSize-blockSize/2+1),...
                ceil(blockSize/2+1):ceil(2*blockSize-blockSize/2+1));
            
            [max_c, imax] = max(abs(x(:)));
            [ypeak, xpeak] = ind2sub(size(x),imax(1));
            
            rowMove(indRowSave,indColSave,indFrames) = ...
                (ypeak-size(currentBlock,1));
            colMove(indRowSave,indColSave,indFrames) = ...
                (xpeak-size(currentBlock,2));
            
  
            
            % Check to see if motion detected is out of bounds
            
            ctotal((indRow):(indRow+blockSize),...
                (indCol):(indCol+blockSize),indFrames) = y;
            
            
            

        end
    end
   
    
end

magMove = sqrt(rowMove.^2 + colMove.^2);
angMove = tand(rowMove./colMove);


end





