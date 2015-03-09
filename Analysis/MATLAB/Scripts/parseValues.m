index = 1;
A225 = zeros(192,1);
A300 = zeros(192,1);
A350 = zeros(192,1);
A400 = zeros(192,1);
A450 = zeros(192,1);
S = zeros(192,1); 
F = zeros(192,1);
L = zeros(192,1);

for i = 1:99:size(l)
    A225(index) = a225(i);
    A300(index) = a300(i);
    A350(index) = a350(i);
    A400(index) = a400(i);
    A450(index) = a450(i);
    F(index) = f(i);
    S(index) = s(i);
    L(index) = l(i);


    index = index + 1;
    
    A225(index) = a225(i+49);
    A300(index) = a300(i+49);
    A350(index) = a350(i+49);
    A400(index) = a400(i+49);
    A450(index) = a450(i+49);
    F(index) = f(i+49);
    S(index) = s(i+49);
    L(index) = l(i+49);
    
    index = index + 1;
    
    A225(index) = a225(i+98);
    A300(index) = a300(i+98);
    A350(index) = a350(i+98);
    A400(index) = a400(i+98);
    A450(index) = a450(i+98);
    F(index) = f(i+98);
    S(index) = s(i+98);
    L(index) = l(i+98);
    
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

plot(L(1:3:end),L(1:3:end)-A225(1:3:end),'ro');
hold on
plot(L(2:3:end),L(2:3:end)-A225(2:3:end),'go');
hold on
plot(L(3:3:end),L(3:3:end)-A225(3:3:end),'bo');

title('RCAL = RFB = 225');
legend('2 KHz','51 KHz','100 KHz');

xlabel('Predicted (Ohms)') % x-axis label
ylabel('Error (Ohms)') % y-axis label

