classdef zCal
    properties
        f = 0;
        z = 0;
        r = 0;
        x = 0;
    end % properties
    methods
        function td = zCal(f, z, r, x)
            if nargin == 4 % Support calling with 0 arguments
                td.f = f;
                td.z = z;
                td.r = r;
                td.x = x;
            else
                error('Constructor must take 4 variables')
            end
        end % constructor

    end% methods
end% classdef