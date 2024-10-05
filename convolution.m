% 定義兩個向量 x 和 h
x = [1 2 0];
h = [3 2 1];

% x = [1 2 1];
% h = [3 2 1];

% x = [5 4 1];
% h = [2 1 1];

% 使用 conv 函數計算卷積
y = conv(x, h);

% 顯示結果
disp('卷積結果:');
disp(y);

% 繪製卷積結果
stem(y, 'filled');
title('x 和 h 的卷積結果');
xlabel('樣本點');
ylabel('值');
grid on;
