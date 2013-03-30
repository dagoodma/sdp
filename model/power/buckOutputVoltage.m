
VO_desired = 5.2; % (V)

% regulated output voltage determined by the external divider resistors RFBT and RFBB
V_out = @(Rfbt, Rfbb) 0.8 * (1 + Rfbt / Rfbb);

% ratio of the feedback resistors for a desired output voltage (Rfbt/Rfbb)
Rratio = @(VO) (VO / 0.8) - 1;

disp(sprintf('Desired (Rfbt / Rfbb) ratio for Vout = 5V: %.3f', Rratio(VO_desired)));

%% Ron

F_sw = @(Vo, Ron) Vo / (1.3*10e-10*Ron);