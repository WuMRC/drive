index = 1;

sizeAllData = size(LCR);
sizeAllData = sizeAllData(1);

singleLCR = zeros(sizeAllData / 99, 1);
singleA = zeros(sizeAllData / 99, 1);

for i = 1:99:sizeAllData - 1
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

plot(diffStep(12:end),diffA(12:end),'bo',diffStep(12:end),diffLCR(12:end),'ro');
h_legend = legend('AD5933', 'LCR');
set(h_legend,'FontSize',20);
set(gca,'FontSize',20); 

ylabel('DIFFERENCE IN RESISTANCE OF CONSECUTIVE STEPS (Ohms)','FontSize',20) % x-axis label
xlabel('STEP NUMBER','FontSize',20) % y-axis label
% singleStep(:,1) = 1:64;
