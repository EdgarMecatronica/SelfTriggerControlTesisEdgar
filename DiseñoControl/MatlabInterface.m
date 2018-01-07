%m-file to adquire ERIKA-FLEX FULL data from the serial port using RS232
%usar esto para cambiar el nombre del puerto usb: sudo ln -s /dev/ttyACM0 /dev/ttyUSB0

clear all
close all

number_of_samples = 500;                 %set the number of adquisitions
delete(instrfind)                         %clean all open ports

%configuring serial port
s = serial('/dev/ttyUSB0');               %creates a matlab object from the serial port
s.BaudRate = 115200;                      %baudrate=115200bps
s.Parity = 'none';                        %no parity
s.DataBits = 8;                           %data sended in 8bits format
s.StopBits = 1;                           %1 bit to stop
s.FlowControl = 'none';                   %no flowcontrol
s.Terminator = 'LF';                      %LineFeed character as terminator
s.Timeout = 1;                            %maximum time in seconds since the data is readed
s.InputBufferSize = 100000;               %increment this value when data is corrupted

q = quantizer('float',[32 8]);            %cast 32bits hex to float

%% Experiment data
try
    disp('Triggering experiment!')
    fopen(s)                                  %open serial port object
    error = 0;
catch
    fclose(s)
    %close all
    disp('Error, port could not be opened!')
    error = 1;
end

if error == 0
    data = zeros(23,1);
    disp('Starting acquisition of data from experiment... ')
    fwrite(s, '1', 'char');
    %figure;
    for n = 1:number_of_samples  %get data
        data = fread(s,23,'char');
        
        t(n,1) = hex2dec([dec2hex(data(2),2) dec2hex(data(3),2) dec2hex(data(4),2) dec2hex(data(5),2)])*25e-9;%25e-9 is 1/Fcy=1/40e6
        r(n,1) = hex2num(q,[dec2hex(data(9),2) dec2hex(data(8),2) dec2hex(data(7),2) dec2hex(data(6),2)]);%joint four bytes, float value
        x1(n,1) = hex2num(q,[dec2hex(data(13),2) dec2hex(data(12),2) dec2hex(data(11),2) dec2hex(data(10),2)]);
        x2(n,1) = hex2num(q,[dec2hex(data(17),2) dec2hex(data(16),2) dec2hex(data(15),2) dec2hex(data(14),2)]);
        u(n,1) = hex2num(q,[dec2hex(data(21),2) dec2hex(data(20),2) dec2hex(data(19),2) dec2hex(data(18),2)]);
        %t(n,1) = bitshift(data(23),24) + bitshift(data(22),16) + bitshift(data(21),8) + data(20); %joint two bytes, unsignded int value
    end
    
    fwrite(s, '0', 'char');
    fclose(s)
    
    %% Sampling data
    try
        fopen(s) %open serial port object
        error = 0;
    catch
        fclose(s)
        %close all
        disp('Error, port could not be opened!')
        error = 1;
    end
    
    if error == 0
        data = zeros(23,1);
        disp('Starting acquisition of data concerning sampling... ')
        %figure;
        fwrite(s, '2', 'char');
        warning ('off','all'); %suppress all warnings
        try
            for n = 1:number_of_samples  %get data
                data = fread(s,23,'char');              
                for i = 2 : 1 : 23
                    a = n*22 - 22 + i - 1;
                    tau(a,1) = hex2dec(dec2hex(data(i),1))/1000;
                end
            end
        catch
            
        end
        
        fclose(s)
        
        %% Plotting
        
        tt = [];
        for i=1:length(t)
            tt = [tt; t(i,1)-t(1,1)];
        end
        
        figure
        subplot(2,1,1)
        
        hold on
        grid on
        plot(tt,r,'g.',tt,x1,'b');
        %axis([max(tt)-3 max(tt) -1.75 1.75]);
        xlim([0 max(tt)]);
        stairs(tt,u,'r') %Plots u as stair
        legend('r','x_1','u')
        xlabel('t(s)')
        ylabel('Voltage (V)')
        title('Acquisition')
        
        subplot(2,1,2)
        hold on
        stem([0; cumsum(tau(1:end-1))],tau,'r')
        tauValid = tau(1:number_of_samples-1)
        plot([0; cumsum(tauValid(1:end))], mean(tauValid),'b*')
        xlim([0 max(tt)]);
        grid on
        disp('Done!')
        
    end
end
