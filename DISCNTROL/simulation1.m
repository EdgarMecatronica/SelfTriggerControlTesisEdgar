
%% Simulation ideal system

%% Initial values

history_tau_k = [];
history_x = [];
history_ref = [];
history_u = [];
history_time = 0;
x = [1; 1]; %initial state
%x_hat = [0.1; 0.1]; % initial observed states
reference = 30;
%%
for n=1:Ns % iterate through time    
    % reference evolution
    if (n>=50)
        reference = 300;
    end
    history_ref = [history_ref, reference];
    
    tau_k = tau_max/(1+(tau_max/eta)*(abs(K_c*(A_c+B_c*K_c)*[x(1)-reference; x(2)]))^alpha); % tau(k)
    tau_k = round(tau_k*1000); % transform into milliseconds in order to round
    tau_k = tau_k/1000; % go back to seconds
    history_tau_k = [history_tau_k, tau_k];
    history_time = [history_time, history_time(end)+tau_k];
    
    % control signal
    Kd_tau_k = lqrd(A_c,B_c,Q_c,R_c,tau_k); % L(tau(k))
    u = Kd_tau_k*[reference-x(1); -x(2)]; % control u(k)
    history_u = [history_u, u];
    
    %dynamics evolution
    [Ad, Bd] = c2d(A_c, B_c, tau_k); % new discrete plant considering tau(k)
    x = Ad*x + Bd*u; % x(k+1)
    history_x = [history_x, x];
end

%% Plotting ideal system

figure

subplot(2,1,1)
grid on, hold on
plot(history_time(1:Ns), history_x(1,:), history_time(1:Ns),...
    history_x(2,:), history_time(1:Ns), history_ref(1,:),'*',...
    history_time(1:Ns), history_u(1,:),'lineWidth', 2);
%%xlim([0 history_time(Ns)]), ylim([-1.1 1.4])
title('Response')
xlabel('Time (sec)')
ylabel('Voltage (V)')
legend('x_{1(k)}','x_{2(k)}','r_{(k)}','u_{(k)}')

subplot(2,1,2)
stem(history_time(1:Ns), history_tau_k)
hold on
plot(history_time(1:Ns), mean(history_tau_k)*ones(size(history_time(1:Ns))),...
    'r','lineWidth', 2)
xlim([0 history_time(Ns)])
title('Sampling sequence')
xlabel('Time (sec)')
ylabel('{\tau}_k (sec)')
legend('\tau_{(k)}','\tau_{av}')

hold off

