index = 1;

sizeAllData = size(LCR);
sizeAllData = sizeAllData(1);

singleLCR = zeros(sizeAllData / 99, 1);
singleA = zeros(sizeAllData / 99, 1);

for i = 49:99:sizeAllData - 1
    % Get value at single frequency (2 KHz)
    singleLCR(index) = LCR(i);
    singleA(index) = A(i);
    index = index+1;
end

sizeSingleData = size(singleLCR);
sizeSingleData = sizeSingleData(1);

diffLCR = zeros(1, sizeSingleData - 1);
diffA = zeros(1, sizeSingleData - 1);
diffStep = 1 : sizeSingleData - 1;

for i = 1 : sizeSingleData - 1
    diffLCR(i) = singleLCR(i + 1) - singleLCR(i);
    diffA(i) = singleA(i + 1) - singleA(i);
end

plot(diffStep,diffA,'bo',diffStep,diffLCR,'ro');
legend('AD5933', 'LCR');

ylabel('Difference in resistance of consecutive steps (Ohms)') % x-axis label
xlabel('Step number') % y-axis label
% singleStep(:,1) = 1:64;
