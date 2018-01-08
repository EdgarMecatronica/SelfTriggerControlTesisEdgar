clear all, close all, clc 


delete(instrfind({'Port'},{'COM3'}));
canal_serie= serial('COM3','BaudRate',9600,'Terminator','CR/LF');
fopen(canal_serie);
%%xlabel('segundos');
%%ylabel('Datos');
%%title('Adquisicion de datos');
%%grid on; hold on;
%%prop = line(nan,nan,'Color','b','LineWidth',1);
A=fscanf(canal_serie,'%f',[1,3]);
clc;
%%disp('Adquisicion de datos de la tarjeta Arduino UNO');
i=1;
NumDatos=5000;
Datos=zeros(NumDatos-1,4);
while i<NumDatos
    A=fscanf(canal_serie,'%f',[1,3]);
   
     Datos(i,1)=A(1,1);
     Datos(i,2)=A(1,2);
     Datos(i,3)=A(1,3);
     Datos(i,4)=i;
     i=i+1;
end

 xlswrite('AngulosAdquiridos.xlsx',Datos);   
fclose(canal_serie);
delete(canal_serie);
clear canal_serie;