R1_225 = csvread('r1_225.csv',0,2,[0,2,6335,2]);
R1_300 = csvread('r1_300.csv',0,2,[0,2,6335,2]);
R1_350 = csvread('r1_350.csv',0,2,[0,2,6335,2]);
R1_400 = csvread('r1_400.csv',0,2,[0,2,6335,2]);
R1_450 = csvread('r1_450.csv',0,2,[0,2,6335,2]);

R4_225 = csvread('r4_225.csv',0,2,[0,2,6335,2]);
R4_300 = csvread('r4_300.csv',0,2,[0,2,6335,2]);
R4_350 = csvread('r4_350.csv',0,2,[0,2,6335,2]);
R4_400 = csvread('r4_400.csv',0,2,[0,2,6335,2]);
R4_450 = csvread('r4_450.csv',0,2,[0,2,6335,2]);

LCR  = csvread('LCR.csv',0,2,[0,2,6335,2]);


index = 1;
r1_225 = zeros(192,1);
r1_300 = zeros(192,1);
r1_350 = zeros(192,1);
r1_400 = zeros(192,1);
r1_450 = zeros(192,1);

r4_225 = zeros(192,1);
r4_300 = zeros(192,1);
r4_350 = zeros(192,1);
r4_400 = zeros(192,1);
r4_450 = zeros(192,1);

S = zeros(192,1);
F = zeros(192,1);
l = zeros(192,1);

for i = 1:99:size(l)
    r1_225(index) = R1_225(i);
    r1_300(index) = R1_300(i);
    r1_350(index) = R1_350(i);
    r1_400(index) = R1_400(i);
    r1_450(index) = R1_450(i);
    
    r4_225(index) = R4_225(i);
    r4_300(index) = R4_300(i);
    r4_350(index) = R4_350(i);
    r4_400(index) = R4_400(i);
    r4_450(index) = R4_450(i);
    
    F(index) = f(i);
    S(index) = s(i);
    l(index) = LCR(i);
    
    
    index = index + 1;
    
    r1_225(index) = R1_225(i+49);
    r1_300(index) = R1_300(i+49);
    r1_350(index) = R1_350(i+49);
    r1_400(index) = R1_400(i+49);
    r1_450(index) = R1_450(i+49);
    
    r4_225(index) = R4_225(i+49);
    r4_300(index) = R4_300(i+49);
    r4_350(index) = R4_350(i+49);
    r4_400(index) = R4_400(i+49);
    r4_450(index) = R4_450(i+49);
    
    F(index) = f(i+49);
    S(index) = s(i+49);
    l(index) = LCR(i+49);
    
    index = index + 1;
    
    r1_225(index) = R1_225(i+98);
    r1_300(index) = R1_300(i+98);
    r1_350(index) = R1_350(i+98);
    r1_400(index) = R1_400(i+98);
    r1_450(index) = R1_450(i+98);
    
    r4_225(index) = R4_225(i+98);
    r4_300(index) = R4_300(i+98);
    r4_350(index) = R4_350(i+98);
    r4_400(index) = R4_400(i+98);
    r4_450(index) = R4_450(i+98);
    
    F(index) = f(i+98);
    S(index) = s(i+98);
    l(index) = LCR(i+98);
    
    index = index + 1;
end
% subplot(2,1,1)
%{
plot(L,L-A225,'ro',L,L-A300,'yo',L,L-A350,'go',L,L-A400,'co',L,L-A450,'bo');

title('RCAL = RFB = 225/300/350/400/450 Ohms');

legend('RFB = RCAL = 225 Ohms','RFB = RCAL = 300 Ohms',...
    'RFB = RCAL = 350 Ohms', 'RFB = RCAL = 400 Ohms',...
    'RFB = RCAL = 450 Ohms');

xlabel('Predicted (Ohms)') % x-axis label
ylabel('Error (Ohms)') % y-axis label
%}
%subplot(2,1,2)

plot(l(1:3:end),l(1:3:end)-r1_225(1:3:end),'ro');
hold on
plot(l(2:3:end),l(2:3:end)-r1_225(2:3:end),'go');
hold on
plot(l(3:3:end),l(3:3:end)-r1_225(3:3:end),'bo');

title('RCAL = RFB = 225');
legend('2 KHz','51 KHz','100 KHz');

xlabel('Predicted (Ohms)') % x-axis label
ylabel('Error (Ohms)') % y-axis label

