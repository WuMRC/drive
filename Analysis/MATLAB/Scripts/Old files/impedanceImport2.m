fileName = '12b';
csv = '''.csv''';
eval([strcat('z',fileName), ' = ', 'csvread(strcat(fileName, ', csv, '),100,3,[100,3,198,3])', ';']);
eval([strcat('t',fileName), ' = ', 'csvread(strcat(fileName, ', csv, '),100,4,[100,4,198,4])', ';']);
eval([strcat('r',fileName), ' = ', strcat('z',fileName), '.*', 'cos(', strcat('t',fileName), ');']);
eval([strcat('x',fileName), ' = ', 'abs(', strcat('z',fileName), '.*', 'sin(', strcat('t',fileName), '));']);
eval([strcat('er',fileName), ' = ', 'rm2 - ', strcat('r',fileName), ';']);
eval([strcat('ex',fileName), ' = ', 'xm2 - ', strcat('x',fileName), ';']);
eval(['plot(f2,', strcat('er',fileName),',''b'',f2,' , strcat('ex',fileName), ',''k'');']);