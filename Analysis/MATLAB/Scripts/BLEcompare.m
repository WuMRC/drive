index = 1;
%b = zeros(64,1);
for i = 1 :size(ble)-1
    if ble(i+1) > (ble(i) + 2);
        b(index) = ble(i);
        index = index + 1;
    end
end

index = 1;
for i = 1 :size(R1_450)-1
    if R1_450(i+1) > (R1_450(i) + 5);
        d(index) = R1_450(i);
        index = index + 1;
    end
end

index = 1;
%b = zeros(64,1);
for i = 1 :size(LCR)-1
    if LCR(i+1) > (LCR(i) + 2);
        e(index) = LCR(i);
        index = index + 1;
    end
end

b = transpose(b);
d = transpose(d);
e = transpose(e);