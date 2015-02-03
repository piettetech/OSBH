%Load File
clear;
file = 'hatching_queen.wav';
[y_whole,Fs,bits] = wavread(file);
Fs
 % pre-processing: normalization (not necessary for many features)
 #{
 if (size(afAudioData,2)> 1)
            afAudioData = afAudioData/max(abs(afAudioData));
        end
 #}
 
for n = 1:10
  y = y_whole(65536*(n-1)+1:65536*(n),1); %extract only mono

  Nsamps = length(y);
  sampletime = Nsamps/Fs;
  time = sampletime*n;
  t = (1/Fs)*(1:Nsamps);          %Prepare time data for plot

  %Hamming Window function
  for i = 1:Nsamps
    y(i) = y(i)* (0.53836 - 0.46164 * cos(2.0 * pi * i / (Nsamps - 1)));
  endfor
  

  %Do Fourier Transform
  y_fft = abs(fft(y));            %Retain Magnitude
  y_fft = y_fft(1:Nsamps/2);      %Discard Half of Points
  f = Fs*(0:Nsamps/2-1)/Nsamps;   %Prepare freq data for plot


  #{
  %Plot Sound File in Time Domain
  figure
  plot(t, y);
  xlabel('Time (s)')
  ylabel('Amplitude')
  title('fft action')
 
 % plot unfiltered fft
  figure
  plot(f, y_fft, "-");
  xlim([0 1000])
  xlabel('Frequency (Hz)')
  ylabel('Amplitude')
  title('Frequency Response Bees unsmoothed')  % frequency distribution
 #}
  
 
  % Construct blurring window.
  gaussFilter = gausswin(20);
  gaussFilter = gaussFilter / sum(gaussFilter); % Normalize.

  % Do the blur.
  y_fil = conv(y_fft, gaussFilter);
  
  %calculate pitches
  [pks, idx, extra] = findpeaks(y_fil,"MinPeakDistance",50,"MinPeakWidth",4);
  
    %Plot Sound File in Frequency Domain
  
  figure
  plot(f,y_fil(1:Nsamps/2),f(idx),y_fil(idx),'.m')
  xlim([0 1500])
  xlabel('Frequency (Hz)')
  ylabel('Amplitude ')
  title({'Frequency Response Bees, time (sec): ';num2str(time)})  % frequency distribution
  
  
  %fill up the data cell array
    %calculate timeslot
    data{n,1} = time;
    data{n,2} = f(idx)';
    data{n,3} = pks;  
    
endfor
  save frequ_data.mat data;
  %save data cell array
  %append_save("frequ_data.mat","-text",{"data", data});
  
  % create virtual spectrum
  spectrum = zeros(2000,1);
  
  for n = 1:rows(data)
    for i = 1:length(data{n,2})
     spectrum(round(data{n,2}(i))) = spectrum(round(data{n,2}(i))) + data{n,3}(i);
    endfor
  endfor
  

  %Plot virtual spectrum
  figure
  plot(1:2000,spectrum);
  xlim([0 1500])
  xlabel('frequ')
  ylabel('magnitude total')
  title('Frequency Response Bees') 