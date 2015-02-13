classdef zValues
    properties
        z = 0;
        r = 0;
        x = 0;
    end % properties
    methods
        function td = zValues(z, r, x)
            if nargin == 3 % Support calling with 0 arguments
                td.z = z;
                td.r = r;
                td.x = x;
            else
                error('Constructor must take 3 variables')
            end
        end % constructor
    end% methods
end% classdef