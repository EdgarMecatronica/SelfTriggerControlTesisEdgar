clear all, clc, close all

T=0.0222;
N=4999
num=[2.075e09]
dem=[1 3.618e05 2.065e09]
FTD=tf(num,dem)
FTD=tf1
%%FTD=d2c(tf_d)
tfss=ss(FTD)

A=[-7.446 -5.705;
    4 0];
B=[2;0];
C=[0 2.841]
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
