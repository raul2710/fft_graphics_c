//Exemplo de uso de FFT - componentes de um sinal
//----------------------------------
// sinal amostrado a 1000hz  contendo frequencias "puras"
// em 50 e 70 Hz
amostragem=1000;
t = 0:1/amostragem:0.6;
N=size(t,'*'); //numero de amostras
s=sin(2*%pi*50*t)+sin(2*%pi*200*t+%pi/4)+grand(1,N,'nor',0,1);

y=fft(s);

//o sinal s é real, entao a FFT é o conjugado simetrico por isso usamos somente a primeira parte (N/2)
//
f=amostragem*(0:(N/2))/N; //vetor com os valores de frequencia
n=size(f,'*')
clf()
plot(f,abs(y(1:n)))
