classdef zValues
    properties
        z = 0; % Impedance magnitude
        r = 0; % Resistance
        x = 0; % Reactance 
    end % properties
    methods
        function td = zValues(z, r, x)
            if nargin == 3 % Support calling with only 3 arguments
                td.z = z;
                td.r = r;
                td.x = x;
            else
                error('Constructor must take 3 variables')
            end
        end % constructor
    end% methods
end% classdef