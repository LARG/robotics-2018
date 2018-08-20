% all params are now read from params.mat
load params.mat
%dt = 0.01 % s
%iu_height = 0.35 % m
%% Constants used to build weighting matrices
%DEG_T_RAD = pi / 180
%gyro_noise_rms = 3 * DEG_T_RAD; % (deg/sec)
%acc_noise_rms = 1; % (m/s^2)
%motion_rms = 1; % (rad/s^3)
%gyro_drift_rms = 0.0001; % rad/sample

A=[1 dt dt^2/2 0;
   0 1  dt     0;
   0 0  1      0;
   0 0  0      1];
B=[dt^3/6 0;
   dt^2/2 0;
   dt     0;
   0      1];
C=[9.81 0 iu_height 0;
   0    1 0         1];
Cz=[1 0 0 0];
Cz1=[0 1 0 0];

% Weighting Matrices
Q=B*diag([motion_rms gyro_drift_rms].^2)*B';
R=diag([acc_noise_rms gyro_noise_rms].^2);

% Now compute Optimal Filter gain
L=dlqe(A,eye(size(A)),C,Q,R);
%format('long')
%disp(L)

save -6 results.mat A B C Cz Cz1 Q R L
