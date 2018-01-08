clear all, clc, close all

T=0.0222;
N=4999
num=[1.55e04]
dem=[1 79.48 514.5]
FTD=tf(num,dem)

tfss=ss(FTD)

A=[-79.48,-16.08;
    32,0];
B=[16;0];
C=[0 30.45]
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
