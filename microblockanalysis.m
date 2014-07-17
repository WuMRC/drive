


[nBlockRows, nBlockCols, nFrames] = size(image_roiNORM);

blockSize = 20;
% nBlockRows = floor(nBlockRows/blockSize);
% nBlockCols = floor(nBlockCols/blockSize);

currentFrameData = image_roiNORM(:,:,1);
nextFrameData = image_roiNORM(:,:,2);


for indRow = 1:blockSize:nBlockRows-blockSize
    for indCol = 1:blockSize:nBlockRows-blockSize
        
        indRowOffset = indRow + blockSize/2;
        indColOffset = indCol + blockSize/2;
        
        currentBlock = currentFrameData(...
            (indRowOffset-blockSize/2):(indRowOffset+blockSize/2),...
            (indColOffset-blockSize/2):(indColOffset+blockSize/2));
        nextBlock = nextFrameData(...
            (indRowOffset-blockSize/2):(indRowOffset+blockSize/2),...
            (indColOffset-blockSize/2):(indColOffset+blockSize/2));
        
        indRowSave = (indRow+blockSize-1)/blockSize;
        indColSave = (indCol+blockSize-1)/blockSize;
        x = normxcorr2(currentBlock,nextBlock);
        y = x((blockSize/2):(2*blockSize-blockSize/2),...
            (blockSize/2):(2*blockSize-blockSize/2));

        [max_c, imax] = max(abs(x(:)));
        [ypeak, xpeak] = ind2sub(size(x),imax(1));
        
        rowMove(indRowSave,indColSave) = (ypeak-size(currentBlock,1));
        colMove(indRowSave,indColSave) = (xpeak-size(currentBlock,2));
        
        corr_offset = [(ypeak-size(currentBlock,1)),...
            (xpeak-size(currentBlock,2))];
        
        
        ctotal((indRow):(indRow+blockSize),...
            (indCol):(indCol+blockSize)) = y;
        
        
%         x((indRowOffset-blockSize):(indRowOffset+blockSize),...
%          (indColOffset-blockSize):(indColOffset+blockSize)) ...
%          = normxcorr2(currentBlock,nextBlock);
     
%         ctotal((indRowOffset-blockSize/2):(indRowOffset+blockSize/2),...
%             (indColOffset-blockSize/2):(indColOffset+blockSize/2)) = ...
%             x((indRowOffset-blockSize/2):(indRowOffset+blockSize/2),...
%             (indColOffset-blockSize/2):(indColOffset+blockSize/2));
        
%         [max_c, imax] = max(abs(c(:,:,indRowSave,indColSave)));
%         [ypeak, xpeak] = ind2sub(size(c),imax(1));
%         corr_offset(:,:,indRowSave,indColSave) = [(ypeak-size(currentBlock,1)), ...
%             (xpeak-size(currentBlock,2))];


%         normxcorr2(currentFrameData)
    end
end

% figure, imagesc(ctotal)

[xx,yy] = meshgrid(indRow,indCol);
figure,imshow(image_roiNORM(:,:,1))
hold on, quiver(xx,yy,rowMove,colMove)

clear ctotal

%%
h = waitbar(0 ,'Progress');
frameIncrement = 1;
total = nFrames-1;
for ind = 1:total
    
    currentFrameData = image_roiNORM(:,:,ind);
    nextFrameData = image_roiNORM(:,:,ind+1);
    
    c(:,:,ind) = normxcorr2(currentFrameData,nextFrameData);
    
    prog = ind/total;
    waitbar(prog,h,'Progress')
    
end
close(h)


