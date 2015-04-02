classdef zValues
    properties
        r1 = 0; % r1 value
        r2 = 0; % r2 value
        c = 0; % capacitance value
        f = 0; % frequency magnitude
        z = 0; % Impedance magnitude
        p = 0; % phase angle
    end % properties
    methods
        function td = zValues(r1, r2, c, z, p)
            if nargin == 5 % Support calling with only 5 arguments
                td.r1 = r1;
                td.r2 = r2;
                td.c = c;
                td.z = z;
                td.p = p;
            else
                error('Constructor must take 5 variables')
            end
        end % constructor
        function td = getRX(zValue)
            nOfFrequencyValues = size(zValue.f);
            nOfFrequencyValues = nOfFrequencyValues(1);
            
            for index = 1 : nOfFrequencyValues
                
            end
            
        end % getRX
    end% methods
end% classdef