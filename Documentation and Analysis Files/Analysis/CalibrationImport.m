fileName = '1';
fileNameAD5933 = '1a';
fileNameLCR = '1l';
csv = '''.csv''';
eval([strcat('z',fileNameAD5933), ' = ', 'csvread(strcat(fileNameAD5933, ', csv, '),0,3,[0,3,98,3])', ';']);
eval([strcat('t',fileNameAD5933), ' = ', 'csvread(strcat(fileNameAD5933, ', csv, '),0,4,[0,4,98,4])', ';']);
eval([strcat('f',fileName), ' = ', 'csvread(strcat(fileNameAD5933, ', csv, '),0,5,[0,5,98,5])', ';']);
eval([strcat('r',fileNameAD5933), ' = ', strcat('z',fileNameAD5933), '.*', 'cos(', strcat('t',fileNameAD5933), ');']);
eval([strcat('x',fileNameAD5933), ' = ', 'abs(', strcat('z',fileNameAD5933), '.*', 'sin(', strcat('t',fileNameAD5933), '));']);

eval([strcat('z',fileNameLCR), ' = ', 'csvread(strcat(fileNameLCR, ', csv, '),1,0,[1,0,99,0])', ';']);
eval([strcat('t',fileNameLCR), ' = ', 'csvread(strcat(fileNameLCR, ', csv, '),1,1,[1,1,99,1])', ';']);
eval([strcat('r',fileNameLCR), ' = ', strcat('z',fileNameLCR), '.*', 'cos(', strcat('t',fileNameLCR), ');']);
eval([strcat('x',fileNameLCR), ' = ', 'abs(', strcat('z',fileNameLCR), '.*', 'sin(', strcat('t',fileNameLCR), '));']);

eval([strcat('er',fileName), ' = ', strcat('r',fileNameLCR), ' - ', strcat('r',fileNameAD5933), ';']);
eval([strcat('ex',fileName), ' = ', strcat('x',fileNameLCR), ' - ', strcat('x',fileNameAD5933), ';']);
eval(['plot(f', fileName, ',', strcat('er',fileName),',''b'',f', fileName, ',' , strcat('ex',fileName), ',''k'');']);