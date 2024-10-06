% 定義兩個向量 x 和 h
x = [1 2 0];
h = [3 2 1];

% x = [1 2 1];
% h = [3 2 1];

% x = [5 4 1];
% h = [2 1 1];

% 顯示輸入的向量 x 和 h
disp('輸入向量 x:');
disp(x);

disp('輸入向量 h:');
disp(h);

% 使用 conv 函數計算卷積
y = conv(x, h);

% 顯示卷積結果
disp('卷積結果 y:');
disp(y);

% 繪製 x, h 和卷積結果 y
figure;

% 第一個圖：顯示向量 x
subplot(3, 1, 1);
stem(x, 'filled');
title('Input x');
xlabel('Sample points');
ylabel('Value');
grid on;

% 第二個圖：顯示向量 h
subplot(3, 1, 2);
stem(h, 'filled');
title('Input h');
xlabel('Sample points');
ylabel('Value');
grid on;

% 第三個圖：顯示卷積結果 y
subplot(3, 1, 3);
stem(y, 'filled');
title('Output y');
xlabel('Sample points');
ylabel('Value');
grid on;
