function serial_callback(obj, event)
%INSTRCALLBACK Display event information for the event.
%
%   INSTRCALLBACK(OBJ, EVENT) displays a message which contains the 
%   type of the event, the time of the event and the name of the
%   object which caused the event to occur.  
%
%   If an error event occurs, the error message is also displayed.  
%   If a pin event occurs, the pin that changed value and the pin 
%   value is also displayed. 
%
%   INSTRCALLBACK is an example callback function. Use this callback 
%   function as a template for writing your own callback function.
%
%   Example:
%       s = serial('COM1');
%       set(s, 'OutputEmptyFcn', {'instrcallback'});
%       fopen(s);
%       fprintf(s, '*IDN?', 'async');
%       idn = fscanf(s);
%       fclose(s);
%       delete(s);
%

%   MP 2-24-00
%   Copyright 1999-2011 The MathWorks, Inc. 
%   $Revision: 1.1.6.2 $  $Date: 2011/05/13 18:06:14 $
%   Modifed by Dr. GE

switch nargin
case 0
   error(message('instrument:instrument:instrcallback:invalidSyntaxArgv'));
case 1
   error(message('instrument:instrument:instrcallback:invalidSyntax'));
case 2
   if ~isa(obj, 'instrument') || ~isa(event, 'struct')
      error(message('instrument:instrument:instrcallback:invalidSyntax'));
   end   
   if ~(isfield(event, 'Type') && isfield(event, 'Data'))
      error(message('instrument:instrument:instrcallback:invalidSyntax'));
   end
end  

% Determine the type of event.
EventType = event.Type;

% Determine the time of the error event.
EventData = event.Data;
EventDataTime = EventData.AbsTime;
   
% Create a display indicating the type of event, the time of the event and
% the name of the object.
name = get(obj, 'Name');
fprintf([EventType ' event occurred at ' datestr(EventDataTime,13),...
	' for the object: ' name '.\n']);

% Display the error string.
if strcmpi(EventType, 'error')
	fprintf([EventData.Message '\n']);
end

% Display the pinstatus information.
if strcmpi(EventType, 'pinstatus')
    fprintf([EventData.Pin ' is ''' EventData.PinValue '''.\n']);
end

% Display the trigger line information.
if strcmpi(EventType, 'trigger')
    fprintf(['The trigger line is ' EventData.TriggerLine '.\n']);
end

% Display the datagram information.
if strcmpi(EventType, 'datagramreceived')
    fprintf([num2str(EventData.DatagramLength) ' bytes were ',...
            'received from address ' EventData.DatagramAddress,...
            ', port ' num2str(EventData.DatagramPort) '.\n']);
end

% Display the configured value information.
if strcmpi(EventType, 'confirmation')
    fprintf([EventData.PropertyName ' was configured to ' num2str(EventData.ConfiguredValue) '.\n']);
end

% jobs start here,
% global data for data exchange with callback
global nrow;   % row
global ncol;   % col
global irow;   %
global icol;   %
global matrix; % holding the data from sensor
global lbyte;  % last byte already read
global llbyte; % last of last byte
global nrem;
global sync;

%if strcmpi(EventType, 'datagramreceived')
if strcmpi(EventType, 'BytesAvailable')
    fprintf('[debug] callback begin to deal with %4.2f bytes data ...\n', obj.BytesAvailable);
    % read the input buffer
    if  obj.BytesAvailable > 0

        [A, count] = fread(obj, obj.BytesAvailable, 'uchar');
        fprintf('[debug] read %4.2f bytes data from input buffer, sync = %4.2f\n', count, sync);
        % parsing the byte stream
        n = 1;
        while ( n <= count )
            if A(n) == 153  && llbyte == 153      % that's 0x99 xx 0x99, sync code found
                fprintf('[debug] two 153 found, now synced \n');
                sync = 1;
                irow = lbyte;
                nrem = 0;          % reset the counter
            elseif lbyte == 153 && llbyte == 153           % counting
                sync = 1;            % reset
                irow = A(n);
                nrem = 0;
            elseif lbyte == 153 && A(n) == 153 
                sync = 1;
                nrem = 0;
            elseif sync == 1
                if nrem == 0
                    icol = A(n);
                    nrem = nrem + 1;
                elseif nrem == 1
                    nrem = nrem + 1;
                elseif nrem == 2
                    matrix(irow, icol) = (lbyte*256 + A(n)) * 5 / 32768;
                    fprintf('irow = %2.0f, icol = %2.0f, v = %3.2f\n', irow, icol, matrix(irow, icol));
                    nrem = 0;
                end
            end
            llbyte = lbyte;
            lbyte  = A(n); 
            n = n + 1;
        end   % end of while...
        fprintf('[debug] sync = %4.2f, nrem = %4.2f \n', sync, nrem);

    end
end 

% draw the first image
subplot(1, 2, 1);
colormap(1 - gray);
imagesc(matrix, [0 1.8]); %range for data display 0V to 1.8V.
colorbar; 
% draw the mesh
subplot(1, 2, 2);
mesh(1:ncol, 1:nrow, matrix);

%figure;
%colormap(1-gray);
%imagesc(data,[0 1.8]); colorbar; %range for data display 0V to 1.8V. These numbers can be modified.
%figure;
%mesh(1:Ncol,1:Nrow,data);