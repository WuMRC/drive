function [rho_c]=corr2D(IX,IY,filt,krnl_limits,srch_limits,pos,flags)
%CORR2D Compute the 2D correlation matrix
%       [RHO]=corr2D(IX,IY,FILT,KRNL_LMT,SRCH_LMT,POS,FLAGS)
%       where:
%             IX = Data matrix 1 (for correlating)
%             IY = Data matrix 2 (to search in when correlating)
%           FILT = Filter matrix (must be odd length in both dimensions)
%       KRNL_LMT = 2 element vector [row col] of the kernel size limits
%                    (kernel size = 2*KRNL_LMT(1)+1 by 2*KRNL_LMT(2)+1)
%       SRCH_LMT = 2 element vector [row col] of the search size limits
%                    (search size = 2*SRCH_LMT(1)+1 by 2*SRCH_LMT(2)+1)
%            POS = 2 element vector [row col] of the position in X_IN to
%                    compute calculations (0 relative)
%          FLAGS = String with flag(s)
%                    ('n'=normalize     [default],'u'=don't normalize,
%                     'm'=subtract mean [default],'c'=don't subtract mean)
%                    So correlation coeff (i.e. normalized cross covariance)
%                                       => 'nm' [default]
%                    strict correlation => 'uc'
%
%            RHO = Correlation coefficient array
%
%       [RHO,RHO_N,RHO_M,RHO_C]=corr2D(...) is the same as above, except
%            that RHO   = normalized cross covariance
%                 RHO_N = normalized cross correlation (no mean subtraction)
%                 RHO_M = strict cross covariance  (no normalization)
%                 RHO_C = strict cross correlation (no mean subtr. or norm.)
%
%       [No Guarantees. M. Lubinski]

% !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
% The above help is for the mex file.
% The M-code below has NOT BEEN UPDATED
% !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

% Check inputs
if (nargin < 6)
  error('Not enough inputs.');
elseif (nargin > 6)
  if (~isstring(flags))
    error('Flags must be a string')
  end
end
if (length(krnl_limits(:)) < 2)
  error('Kernel limits must be [row column]');
end
if (length(srch_limits(:)) < 2)
  error('Search limits must be [row column]');
end
if (length(pos(:)) < 2)
  error('Position must be [row column]');
end

flag_norm=1;
flag_mean=1;
if (nargin > 6)
  for (i=1:length(flags))
    if (~any(flag(i)=='unmc'))
      error(['Unknown flag ' flag(i)])
    end
  end
  flag_norm=~any(flags=='u');
  flag_mean=~any(flags=='c');
else


% Compute needed values from inputs
kyl=krnl_limits(1);
kxl=krnl_limits(2);
syl=srch_limits(1);
sxl=srch_limits(2);
Py=pos(1)-1;
Px=pos(2)-1;
fyl=fix(size(filt,1)/2);
fxl=fix(size(filt,2)/2);
if ((2*fxl+1) ~= size(filt,1)) | ((2*fyl+1) ~= size(filt,2))
  error('Filter matrix must be odd length in both dimensions.');
end

% Compute indecies
% disp('Initializing...');
index_y=Py+(-kyl:kyl);
index_x=Px+(-kxl:kxl);

% Save room
rho_c=zeros(2*syl+1,2*sxl+1);		% Complex
rho_num=zeros(2*syl+1,2*sxl+1);		% Complex Not normalized

% Compute with filtering
% disp('Computing...')
for fr=(-fyl):fyl
  for fc=(-fxl):fxl

    % Get X & compute things exclusive to X
    X=IX(index_y+fr,index_x+fc);
    X=X-mean(X(:));
    XX=X.*conj(X);
    XP = sum(sum(XX));
    
    % Search
    for sr=(-syl):syl
      for sc=(-sxl):sxl
	Y=IY(index_y+sr+fr,index_x+sc+fc);
	Y=Y-mean(Y(:));
	
	YY= Y.*conj(Y);
	YP = sum(sum(YY));

	XY= X.*conj(Y);
	
	rho_1c = sum(sum(XY))/sqrt(XP * YP);
	rho_c(sr+syl+1,sc+sxl+1) = rho_c(sr+syl+1,sc+sxl+1) + ...
	    rho_1c * filt(fr+fyl+1,fc+fxl+1);
	
      end
    end
  end
end

end

