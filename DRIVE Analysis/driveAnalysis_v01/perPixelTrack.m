function [posNew] = perPixelTrack(currentFrameData, nextFrameData, filt,...
    kernel, search, posOriginal)
% Function to track frame-to-frame motion


% Calculate the correlation
rho_c = corr2D(currentFrameData, nextFrameData, filt, ...
    kernel, search, posOriginal);
rho_n = rho_c./max(max(rho_c));

% Calculate movement based on max correlation
[rowMove, colMove] = find(rho_n == max(max(rho_n)));

% Need to compenate for drift and net motion of the image
%         cornerOffset = 20;

%
%         [rho_c(:,:,ind)] = corr2D(currentFrameData, nextFrameData, filt, ...
%             [rowKernel, colKernel], [rowSearch, colKernel], posNew);
%         rho_n(:,:,ind) = rho_c(:,:,ind)./max(max(rho_c(:,:,ind)));

rowMove = rowMove - (search(1) + 1);
colMove = colMove - (kernel(2) + 1);

% rowMove_total = sum(rowMove(:,indPoints));
% colMove_total = sum(colMove(:,indPoints));

poiRow = posOriginal(1) + rowMove;%_total;
poiCol = posOriginal(2) + colMove;%_total;

posNew = [poiRow, poiCol];

end



%         % Calculate the correlation
%         [rho_c(:,:,ind)] = corr2D(currentFrameData, nextFrameData, filt, ...
%             [rowKernel, colKernel], [rowSearch, colKernel], posNew);
%         rho_n(:,:,ind) = rho_c(:,:,ind)./max(max(rho_c(:,:,ind)));
%    
%         % Calculate movement based on max correlation
%         [rowMove(ind,indPoints), colMove(ind,indPoints)] = find(rho_n(:,:,ind) ... 
%             == max(max(rho_n(:,:,ind))));
%         
%         % Need to compenate for drift and net motion of the image
% %         cornerOffset = 20;
%  
% %         
% %         [rho_c(:,:,ind)] = corr2D(currentFrameData, nextFrameData, filt, ...
% %             [rowKernel, colKernel], [rowSearch, colKernel], posNew);
% %         rho_n(:,:,ind) = rho_c(:,:,ind)./max(max(rho_c(:,:,ind)));
%         
%         
%     
%         rowMove(ind,indPoints) = rowMove(ind,indPoints) - (rowSearch + 1);
%         colMove(ind,indPoints) = colMove(ind,indPoints) - (colKernel + 1);
%     
%         rowMove_total = sum(rowMove(:,indPoints));
%         colMove_total = sum(colMove(:,indPoints));
%     
%         poiRow(ind,indPoints) = posOriginal(1) + rowMove_total;
%         poiCol(ind,indPoints) = posOriginal(2) + colMove_total;
%     
%         posNew = [poiRow(ind,indPoints), poiCol(ind,indPoints)];
%     
%         % Track single pixel in image
%         imageTrack(poiRow(ind,indPoints),poiCol(ind,indPoints),ind) = 400;