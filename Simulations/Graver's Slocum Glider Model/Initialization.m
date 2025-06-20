%% Set Glider Parameters

clear;                      % Clear workspace
clc;                        % Clear screen              

%% Set glider trajectory and other inputs


Xi_d = deg2rad(-25);        % Glider trajectory initial desired glide path angle
V_d = 0.3;                  % Glider speed in the vertical plane
rp3 = 0.05;                 % Fixed position of primary moving mass in body coordinate along e3 axis
rs3 = 0.00;                 % Fixed position of secondary moving mass in body coordinate along e3 axis
rb1 = 0.00;                 % Fixed position of ballast mass in body coordinate along e1 axis
rb3 = 0.00;                 % Fixed position of ballast mass in body coordinate along e3 axis
u_ballast_rate = 0.0250;    % Ballast rate input (kg/s) (Magnitude only, sign is assigned later)
phi = 0;                    % Glider roll angle
yaw = 0;                    % Glider yaw angle
theta0 = 0.4;               % Initial pitch angle

%% Constants

g = 9.816;                  % Acceleration due to gravity
I3 = eye(3);                % Identity matrix (3x3)
Z3 = zeros(3);              % Zero matrix (3x3)

i = [1 0 0]';               % Unit vector along earth frame x-axis
j = [0 1 0]';               % Unit vector along earth frame y-axis
k = [0 0 1]';               % Unit vector along earth frame z-axis

%% Mass Properties

mh = 40;                        % Hull mass
mbar = 9;                       % Primary moving inernal mass
ms = 0;                         % Secondary moving inernal mass
mb = 1.0;                       % Variable ballast mass     
mw = 0;                         % Fixed point mass
mv = mh + mw + ms + mb + mbar;  % Total vehicle mass
m = 50;                         % Displacement
m0 = mv - m;                    % Buoyancy

mf1 = 5;                        % Added mass term
mf2 = 60;                       % Added mass term
mf3 = 70;                       % Added mass term     
Mf = diag([mf1, mf2, mf3]);     % Added mass matrix

M = mh*I3 + Mf;                 % Total mass
Minv = inv(M);
m1 = M(1,1);    
m2 = M(2,2);
m3 = M(3,3);

j1 = 4;                         % Inertia term  
j2 = 12;                        % Inertia term
j3 = 11;                        % Inertia term
J = diag([j1, j2, j3]);         % Total inertia
Jinv = inv(J);

%% Force and Moment Coefficients 

KL = 132.5;                 % Lift coefficient
KL0 = 0;                    % Lift coefficient

KD = 25;                    % Drag Coefficient
KD0 = 2.15;                 % Drag Coefficient

KM = -100;                  % Moment Coefficient
KM0 = 0;                    % Moment Coefficient

KOmega1_2 = -50;            % Rotational Damping Coefficient
KOmega2_2 = -50;            % Rotational Damping Coefficient

%% Admissible values of xi_d

lim1 = rad2deg(atan(2*(KD/KL)*((KL0/KL) + nthroot(((KL0/KL)^2) + (KD0/KD), 2))));
lim2 = rad2deg(atan(2*(KD/KL)*((KL0/KL) - nthroot(((KL0/KL)^2) + (KD0/KD), 2))));

%% Desired value of glide angle
xi_d = Xi_d(1);

%% Set glide direction & ballast rate
% Glide direction (U = Upwards, D = Downwards)
if xi_d > 0
    glide_dir = 'U';
    disp('Glider direction: Upward');
    u_ballast_rate = -abs(u_ballast_rate);          % Negative ballast rate for upward glide
elseif xi_d < 0
    glide_dir = 'D';
    disp('Glider direction: Downward');
    u_ballast_rate = abs(u_ballast_rate);           % Positive ballast rate for downward glide
end

%% Desired angle of attack
alpha_d = (1/2)*(KL/KD)*(tan(xi_d))*(-1 + nthroot(1 - 4*(KD/(KL^2))*(cot(xi_d))*(KD0*cot(xi_d) + KL0),2));
%% Desired velocity in body frame
v1_d = V_d*cos(alpha_d);
v3_d = V_d*sin(alpha_d);
%% Desired ballast mass
mb_d = (m - mbar - mh - ms -mw) + (1/g)*( (-sin(xi_d))*(KD0 + KD*(alpha_d^2)) + (cos(xi_d))*(KL0 + KL*(alpha_d)))*(V_d^2);
%% Desired position of longitudinal moving masses

lambda = 1;
theta_d = alpha_d + xi_d;

rp1_d = (1/(mbar + lambda*ms))*(-mb_d*rb1 -tan(theta_d)*(mbar*rp3 + ms*rs3 + mb*rb3) + (1/(g*cos(theta_d)))*((mf3 - mf1)*v1_d*v3_d + (KM0 + KM*alpha_d)*(V_d^2)));
rs1_d = lambda*rp1_d;
 