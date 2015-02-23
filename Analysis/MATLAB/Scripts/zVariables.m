classdef zVariables
    properties
        f = 0; % AC frequency
        r1 = 0; % resitance in parallel with r2 and c
        r2 = 0; % resistance in series with capacitor "c"
        c = 0; % capacitor in series with resistor "r1"
        key = 'key';
    end % properties
    
    methods
        function td = zVariables(f, r1, r2, c, key)
            if nargin == 5 % Support calling with only 5 arguments
                td.f = f;
                td.r1 = r1;
                td.r2 = r2;
                td.c = c;
                td.key = key;              
            else
                error('Constructor must take 4 variables')
            end
        end % constructor
    end% methods
end% classdef