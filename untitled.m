% Input signals
x = [1, 2, 0];
h = [3, 2, 1];

% Length of the signals
len_x = length(x);
len_h = length(h);
len_y = len_x + len_h - 1; % Length of the output signal

% Zero-pad the signals
x = [x, zeros(1, len_y - len_x)];
h = [h, zeros(1, len_y - len_h)];

% Initialize the output signal
y = zeros(1, len_y);

% Step-by-step convolution process visualization
figure;
for i = 1:len_y
    % Compute the convolution at each step
    for j = 1:i
        y(i) = y(i) + x(j) * h(i-j+1);
    end
    
    % Plot the current step result with signal alignment
    subplot(len_y, 1, i); % Create a subplot for each step
    stem(0:len_y-1, x, 'b', 'filled'); % Plot signal x in blue
    hold on;
    stem(0:len_y-1, circshift(h, [0, i-1]), 'r', 'filled'); % Plot shifted signal h in red
    title(['Step ' num2str(i) ': Convolution Calculation']);
    xlabel('Position');
    ylabel('Value');
    legend('x[n]', 'Shifted h[n-k]');
    grid on;
    hold off;
end

% Display the final result
figure;
stem(0:len_y-1, y, 'filled');
title('Final Convolution Result');
xlabel('Position');
ylabel('Value');
grid on;
