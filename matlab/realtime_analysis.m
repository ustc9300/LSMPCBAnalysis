%
%
% A real time PCB data analynizer, created by Dr.GE
% 
clear;
%
% create a serial comm object
%
% for Linux 64   serial('/dev/ttyS0');
% for MacOSX 64  serial('/dev/tty.KeySerial1');
% for Windows 64 serial('com1');
% with parameters: 
% serial('com1','baudrate',9600,'parity','none','databits',8,'stopbits',1)
%
%  '/dev/tty.xxxx', 8N1@9600
%
%s = serial('/dev/tty.Bluetooth-Incoming-Port', 'baudrate', 9600, 'parity', 'none', 'databits',8, 'stopbits', 1);
s = serial('com2', 'baudrate', 9600, 'parity', 'none', 'databits',8, 'stopbits', 1);
%
% add more properties, note that the serial port's properties cannot be modifed after opened.
%
%    set(s)
%    ByteOrder: [ {littleEndian} | bigEndian ] 
%    BytesAvailableFcn
%    BytesAvailableFcnCount
%    BytesAvailableFcnMode: [ {terminator} | byte ]
%    ErrorFcn
%    InputBufferSize
%    Name
%    OutputBufferSize
%    OutputEmptyFcn
%    RecordDetail: [ {compact} | verbose ]
%    RecordMode: [ {overwrite} | append | index ]
%    RecordName
%    Tag
%    Timeout
%    TimerFcn
%    TimerPeriod
%    UserData
%
%    SERIAL specific properties:
%    BaudRate
%    BreakInterruptFcn
%    DataBits
%    DataTerminalReady: [ {on} | off ]
%    FlowControl: [ {none} | hardware | software ]
%    Parity: [ {none} | odd | even | mark | space ]
%    PinStatusFcn
%    Port
%    ReadAsyncMode: [ {continuous} | manual ]
%    RequestToSend: [ {on} | off ]
%    StopBits
%    Terminator

s.InputBufferSize = 4096;
s.Timeout = 1.0;
% set callback as 'terminate' instead of 'byte"
s.BytesAvailableFcnMode = 'byte';
% set buffer size for signal
s.BytesAvailableFcnCount = 2500;
% set the callback function handle
s.BytesAvailableFcn = @serial_callback;
% not quite sure of the "outputEmpty" signal
s.OutputEmptyFcn = @serial_callback;

% global data for data exchange with callback
global nrow;   % row
global ncol;   % col
global irow;   %
global icol;   %
global matrix; % holding the data from sensor
global lbyte;  % last byte already read
global llbyte; % last of last byte
global nrem;   % bytes in last buffer
global sync;   % if synchronized0

nrow   = 47;
ncol   = 48;
irow   = 0;
icol   = 0;
matrix = zeros(nrow, ncol);
lbyte  = 0;
llbyte = 0;
sync   = 0;
nrem   = 0;

% create a UI component here
figure;

% draw the first image
subplot(1, 2, 1);
colormap(1 - gray);
imagesc(matrix, [0 1.8]); %range for data display 0V to 1.8V.
colorbar; 
% draw the mesh
subplot(1, 2, 2);
mesh(1:ncol, 1:nrow, matrix);

% connect the serial port
fopen(s);
% waiting for anykey to exit
fprintf('[debug] open serial, and waiting for exit\n');
pause;
% disconnect the serial
fclose(s);
% delete the serial comm object in memory
delete(s);
% clear the object in workspace
clear s;