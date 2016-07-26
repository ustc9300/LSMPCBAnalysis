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
if strcmpi(EventType, 'datagramreceived')
    % read the input buffer
    if  obj.BytesAvailable > 0

        [A, count] = fread(obj, obj.BytesAvailable, 'uchar');
        % parsing the byte stream
        n = 1;
        k = 1;
        while ( n <= count )
            if A(n) == 153          % that's 0x99
                if lbyte == 153     % sync code \0x99\0x99 found
                    index = 0;      % reset the index
                    k = 1;          % reset the counter
                elseif index == nrow * ncol % the first 0x99 or start of a line
                    k = 1;                    
                else                % the last byte is the row id
                    k = 1;
                end
            elseif k == 3           % counting
                k = 1;              % reset
                matrix(floor(index/ncol) + 1, mod(index,ncol) + 1) = (lbyte*256 + A(n)) * 5 / 32768;
                index = index + 1;
            elseif k == 2
                k = k + 1;
            elseif k == 1
                k = k + 1;
            end

            llbyte = lbyte;
            lbyte  = A(n); 
            n = n + 1;
        end   % end of while...
    end
end 

%figure;
%colormap(1-gray);
%imagesc(data,[0 1.8]); colorbar; %range for data display 0V to 1.8V. These numbers can be modified.
%figure;
%mesh(1:Ncol,1:Nrow,data);

