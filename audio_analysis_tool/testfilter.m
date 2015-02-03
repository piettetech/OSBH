% Generate sample data.
vector = 5*(1+cosd(1:3:180)) + 2 * rand(1, 60);
plot(vector, 'r-', 'linewidth', 3);
set(gcf, 'Position', get(0,'Screensize')); % Maximize figure.

% Construct blurring window.
windowWidth = int16(5);
halfWidth = windowWidth / 2
gaussFilter = gausswin(5)
gaussFilter = gaussFilter / sum(gaussFilter); % Normalize.

% Do the blur.
smoothedVector = conv(vector, gaussFilter)

% plot it.
hold on;
plot(smoothedVector(halfWidth:end-halfWidth), 'b-', 'linewidth', 3); 