clear all, clc, close all

T=0.0222;
N=4999
num=[73.4]
dem=[1 12.39 7362]
FTD=tf(num,dem)

tfss=ss(FTD)

A=[-12.39 -115;
    64 0];
B=[1;0];
C=[0 1.147]
D=[0]

Q=[1 0;0 1];
S=[1 0;0 1];
R=[1];

Polos=eig(A)
Ttotal=111;
[K] = lqr(A,B,Q,R)

Densidad = Ttotal/N

Tk= Tmax*(1/((Tmax/eta)*(abs(K*(A+B*K)*X)^alfa)+1))
X=[0;0]
U=-K*X
X2=A*X + B*U

Opdens=abs(U)^(2/3)
