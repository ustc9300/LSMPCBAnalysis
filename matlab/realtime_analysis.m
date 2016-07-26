%
%  A real time PCB data analynizer, coded by Dr.GE
% 

% create a serial comm object
%
% Linux® 64   serial('/dev/ttyS0');
% Mac OS X 64 serial('/dev/tty.KeySerial1');
% Windows® 64 serial('com1');
% With parameters :
% serial（’com1’，’baudrate’,9600,’parity’,’none’,’databits’,8,’stopbits’,1）
%
%  '/dev/tty.xxxx', 8N1@9600
%
scom=serial('/dev/tty....', 'baudrate', 9600, 'parity', 'none', 'databits',8, 'stopbits', 1);


