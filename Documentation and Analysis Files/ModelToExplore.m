% A model

Re = 100:100:1000;      % External cellular resistance
Ri = 100:100:1000;      % Internal cellular resistance
Cm = (1:1:10).*10.^-9;   % Cell membrane capacitance
f = (1:100).*1000;

tau = zeros(length(Re),length(Ri),length(Cm));

for indRe = 1:length(Re)
    R0(indRe) = Re(indRe);
    for indRi = 1:length(Ri)
        Rinf(indRe,indRi) = ...
            (Re(indRe) * Ri(indRi)) / (Re(indRe) + Ri(indRi));
        Rrange(indRe,indRi) = R0(indRe) - Rinf(indRe,indRi);
        for indCm = 1:length(Cm)
            tau(indRe,indRi,indCm) = ...
                (Re(indRe)+Ri(indRi)).*Cm(indCm);
            Rm(indRe,indRi,indCm,1:100) = Rinf(indRe,indRi) ...
                + (Rrange(indRe,indRi)...
                ./ (1 + (2*pi * tau(indRe,indRi,indCm) *  f).^2));
            Xm(indRe,indRi,indCm,1:100) = Rrange(indRe,indRi) ...
                * 2*pi * tau(indRe,indRi,indCm) * f ...
                ./ (1+ (2*pi * tau(indRe,indRi,indCm) *  f).^2);
        end
    end
end

%% Plot the R-X model
ReOfInterest = 100;
RiOfInterest = 200;
CmOfInterest = 10.*10.^-9;

indReOfInterest = find(Re == ReOfInterest);
indRiOfInterest = find(Ri == RiOfInterest);
indCmOfInterest = find(Cm == CmOfInterest);

plot(permute(Rm(indReOfInterest,indRiOfInterest,indCmOfInterest,:),...
    [4 3 2 1]),...
    permute(Xm(indReOfInterest,indRiOfInterest,indCmOfInterest,:),...
    [4 3 2 1]))

xlabel('Resistance'), ylabel('Reactance')


%% Find relationships
% Re changing
title('Re changing')
subplot(2,2,4)
hold on
for n = 1:length(Re)
    demoRe(:,n) = permute(Rm(n,indRiOfInterest,indCmOfInterest,:),...
    [4 3 2 1]);
    demoXe(:,n) = permute(Xm(n,indRiOfInterest,indCmOfInterest,:),...
    [4 3 2 1]);
    plot(demoRe(:,n),demoXe(:,n));
end
hold off
xlabel('Resistance'), ylabel('Reactance')


subplot(2,2,1), mesh(demoRe);
xlabel('Re level'), ylabel('Frequency'), zlabel('Resistance'), 
subplot(2,2,2), mesh(demoXe);
xlabel('Re level'), ylabel('Frequency'), zlabel('Reactance'), 
subplot(2,2,3), mesh(1:10,demoRe,demoXe)
xlabel('Re level'), ylabel('Resistance'), zlabel('Reactance'), 


%%
% Ri changing
figure
title('Ri changing')
subplot(2,2,4)
hold on
for n = 1:length(Ri)
    demoRi(:,n) = permute(Rm(indReOfInterest,n,indCmOfInterest,:),...
    [4 3 2 1]);
    demoXi(:,n) = permute(Xm(indReOfInterest,n,indCmOfInterest,:),...
    [4 3 2 1]);
    plot(demoRi(:,n),demoXi(:,n));
end
hold off
xlabel('Resistance'), ylabel('Reactance')


subplot(2,2,1), mesh(demoRi);
xlabel('Ri level'), ylabel('Frequency'), zlabel('Resistance'), 
subplot(2,2,2), mesh(demoXi);
xlabel('Ri level'), ylabel('Frequency'), zlabel('Reactance'), 
subplot(2,2,3), mesh(1:10,demoRi,demoXi)
xlabel('Ri level'), ylabel('Resistance'), zlabel('Reactance'), 




%%

% Cm changing
figure
title('Cm changing')
subplot(2,2,4)
hold on
for n = 1:length(Cm)
    demoCm(:,n) = permute(Rm(indReOfInterest,indReOfInterest,n,:),...
    [4 3 2 1]);
    demoCm(:,n) = permute(Xm(indReOfInterest,indReOfInterest,n,:),...
    [4 3 2 1]);
    plot(demoCm(:,n),demoCm(:,n));
end
hold off
xlabel('Resistance'), ylabel('Reactance')


subplot(2,2,1), mesh(demoCm);
xlabel('Ri level'), ylabel('Frequency'), zlabel('Resistance'), 
subplot(2,2,2), mesh(demoCm);
xlabel('Ri level'), ylabel('Frequency'), zlabel('Reactance'), 
subplot(2,2,3), mesh(1:10,demoCm,demoCm)
xlabel('Ri level'), ylabel('Resistance'), zlabel('Reactance'), 
