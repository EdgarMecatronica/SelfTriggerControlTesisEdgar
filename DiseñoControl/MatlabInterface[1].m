%m-file to adquire arduino data from the serial port using USB
%if the OS is Linux, type on the terminal the following, to change the name
%of the usb port: sudo ln -s /dev/ttyACMx /dev/ttyUSBx, where x is the number
%of the port where the arduino board is attached

try
    clear all
    close all
    
    number_of_samples = 300;                  %set the number of adquisitions
    frameLength = 21;
    
    delete(instrfind)                         %clean all open ports
    
    %configuring serial port
    %s=serial('/dev/ttyUSB0');                %linux
    %s=serial('COM1');                        %windows
    s = serial('/dev/cu.usbmodem1421');       %mac os
    s.BaudRate = 115200;                      %baudrate=115200bps
    s.Parity = 'none';                        %no parity
    s.DataBits = 8;                           %data sended in 8bits format
    s.StopBits = 1;                           %1 bit to stop
    s.FlowControl = 'none';                   %no flowcontrol
    s.Terminator = 'LF';                      %LineFeed character as terminator
    s.Timeout = 3;                            %maximum time in seconds since the data is readed
    s.InputBufferSize = 100000;               %increment this value when data is corrupted
    
    q = quantizer('float',[32 8]);          %cast 32bits hex to float
    
    fopen(s)                                %open serial port object
    %fwrite(s, '1', 'char');
    disp('Waiting for data... ')
    
    %Read data until header character (0x01) is received
    data = zeros(frameLength,1);
    while(data ~= 1)
        data = fread(s,1,'char');
    end
    data = fread(s,frameLength-1,'char');
    
    disp('Beginning data acquisition... ')
    figure;
    for n = 1:number_of_samples  %get data and plot
        data = fread(s,frameLength,'char');
        
        t(n,1) = hex2dec([dec2hex(data(2),2) dec2hex(data(3),2) dec2hex(data(4),2) dec2hex(data(5),2)])*1e-3;%25e-9 is 1/Fcy=1/40e6
        r(n,1) = hex2num(q,[dec2hex(data(6),2) dec2hex(data(7),2) dec2hex(data(8),2) dec2hex(data(9),2)]);%joint four bytes, float value
        x1(n,1) = hex2num(q,[dec2hex(data(10),2) dec2hex(data(11),2) dec2hex(data(12),2) dec2hex(data(13),2)]);
        x2(n,1) = hex2num(q,[dec2hex(data(14),2) dec2hex(data(15),2) dec2hex(data(16),2) dec2hex(data(17),2)]);
        u(n,1) = hex2num(q,[dec2hex(data(18),2) dec2hex(data(19),2) dec2hex(data(20),2) dec2hex(data(21),2)]);
        %t(n,1)=bitshift(data(23),24) + bitshift(data(22),16) + bitshift(data(21),8) + data(20); %joint two bytes, unsignded int value
        
        plot(t,r,'g.',t,x1,'b',t,x2,'m','linewidth',2);
        axis([max(t)-1 max(t) -2.2 2.2]);%DI Plant
        %axis([max(t)-3 max(t) 0 3]);%RCRC Plant
        grid on;
        
        drawnow;
    end
    
    %fwrite(s, '0', 'char');
    fclose(s)
    
    disp('Plotting data... ')
    hold on
    stairs(t,u,'r') %Plots u as stair
    legend('r','x_1','x_2','u','fontsize',15)
    xlabel('t(s)','fontsize',20)
    ylabel('voltage (V)','fontsize',20)
    title('Acquisition','fontsize',20)
catch
    fclose(s)
    close all
    disp('Error!')
end

