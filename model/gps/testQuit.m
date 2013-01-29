breakkey = '5';

for ii = 1:100
   fprintf('Iteration number %g', ii)
   pause(0.5);
   if strcmp(get(gcf,'currentkey'),breakkey)
        disp('break key pressed!');
        break
   end
end