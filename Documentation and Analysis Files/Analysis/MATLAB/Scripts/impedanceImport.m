fileName = '6';
csv = '''.csv''';
eval([strcat('z',fileName), ' = ', 'csvread(strcat(fileName, ', csv, '),112,3,[112,3,222,3])', ';']);
eval([strcat('t',fileName), ' = ', 'csvread(strcat(fileName, ', csv, '),112,4,[112,4,222,4])', ';']);
eval([strcat('r',fileName), ' = ', strcat('z',fileName), '.*', 'cos(', strcat('t',fileName), ');']);
eval([strcat('x',fileName), ' = ', 'abs(', strcat('z',fileName), '.*', 'sin(', strcat('t',fileName), '));']);
eval([strcat('er',fileName), ' = ', 'rm - ', strcat('r',fileName), ';']);
eval([strcat('ex',fileName), ' = ', 'xm - ', strcat('x',fileName), ';']);
eval(['plot(f,', strcat('er',fileName),',''b'',f,' , strcat('ex',fileName), ',''k'');']);