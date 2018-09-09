autoscoper_socket = openConnection('127.0.0.1');

frame = 120;
repeats = 1; % 1 is good for PSO
max_iter = 1000; % Maximum epochs allowed
min_lim = -2; % Minimum range; But if the best answer is around that, we look for higher ranges too
max_lim = 2; % Maximum range; But if the best answer is around that, we look for higher ranges too

setFrame(autoscoper_socket,frame) % Go to that frame first

pause(1);
optimizeFrame(autoscoper_socket,frame,repeats,max_iter,min_lim,max_lim)
pause(1);

setFrame(autoscoper_socket,frame) % Refresh the view
