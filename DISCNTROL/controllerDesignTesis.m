%% TWO-THIRDS CONTROLLER
clear all, close all, clc,

%%A=[-12.39 -115;
%     64 0];
% B=[1;0];
% C=[0 1.147]
% D=[0]
A_c = [-12.39 -115;
    64 0]; %dynamic system definition
B_c = [1;0];
C = [0 1.147];
D = 0;

sys = ss(A_c, B_c, C, D);

%Q_c = 1/(23.8095^2)*eye(size(A_c,1));
Q_c = [0.0025 0; 0 0.0025];
%R_c = 2/23.8095;
R_c = 0.1;
%S = 1e-4*[3, -5; -5, 20];

%% Continuous time controller-gain

K_c = lqr(A_c,B_c,Q_c,R_c) %continuous time control gain
eigenvalues = eig(A_c-B_c*K_c) %stability test

%% Settings
tau_max = 40e-3;
tau_min = 10e-3;
granularity = 1e-3;
beta = 9.4;
alpha = 0.667;
eta = 0.11;
Ns = 100; % Number of samples for simulation
refer = 0.5; %reference for simulation
snr = 55; %signal to noise ratio

%pole1 = -Inf; %poles in continous time for the observer's poles placement
%pole2 = -Inf;
 pole1 = -3+2i;
 pole2 = -3-2i;
% pole1 = -70+10i;
 %pole2 = -70-10i;
% pole1 = -20+5i;
% pole2 = -20-5i;
P_c = [pole1, pole2]
%% guarantee that all sampling periods will be in the range [tau_min, tau_max]

(beta^alpha)/eta
1/tau_min - 1/tau_max

%% Kd(tau_k) for all possible tau_k
Kd_tau_k = [];

for tau_k = tau_min : granularity : tau_max
    Kd_tau_k = [Kd_tau_k; lqrd(A_c,B_c,Q_c,R_c,tau_k)]; %gain matrices
end

%% Calculating functions to fit the controller-gains behaviour
tau_k = [tau_min: granularity: tau_max]';
h = figure;
subplot(2, 1, 1);
%plot(tau_k, Kd_tau_k(:,1), tau_k, Kd_tau_k(:,2));

% new polynomials to fit elements of controller gain matrix along tau_k
K11 = polyfit(tau_k, Kd_tau_k(:,1), 1) %1st element
K12 = polyfit(tau_k, Kd_tau_k(:,2), 1) %2nd element

Kd_tk_1 = polyval(K11, tau_k);
Kd_tk_2 = polyval(K12, tau_k);

hold on
plot(tau_k, Kd_tk_1, 'or', tau_k, Kd_tk_2, 'ob','lineWidth',2);
plot(tau_k, Kd_tk_1, 'Color','r','lineWidth',2);
plot(tau_k, Kd_tk_2, 'Color','b','lineWidth',2);
set(gca, 'FontSize', 24);
title('Controller gains')

xlabel('\tau_k (sec)');
ylabel('K_{d(\tau_k)}')
legend('g_{11}(\tau_h)','g_{12}(\tau_h)','K_{d11}(\tau_k)','K_{d12}(\tau_k)');

hold off

%% Implementation into the microcontroller (C language)

syms A00 A01 A10 A11
syms B00 B10
syms L00 L01
syms x00 x10
syms r

a = [A00 A01; A10 A11];
b = [B00; B10];
l = [L00 L01];
X = [(x00-r); x10];

l*(a+b*l)*X


%%

simulation1
