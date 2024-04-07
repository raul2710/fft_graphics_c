//Frequency components of a signal
//----------------------------------
// build a noides signal sampled at 1000hz  containing to pure frequencies 
// at 50 and 70 Hz
sample_rate=8;
t = 0:1/sample_rate:1;
N=size(t,'*'); //number of samples
s=sin(2*%pi*50*t)+sin(2*%pi*70*t+%pi/4);

y=fft(s);
//the fft response is symmetric we retain only the first N/2 points
f=sample_rate*(0:(N/2))/N; //associated frequency vector
n=size(f,'*')

subplot(2, 1, 1);
plot(s);

subplot(2, 1, 2);
plot(f,abs(y(1:n)))
