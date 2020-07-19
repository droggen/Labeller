sr = 100;
d=1;

fr=[0 5 0 10 0 15 0];

s=[];
l=[];
for fi = 1:length(fr)
    
    stmp = gencoswave(fr(fi),pi/2,sr,d*sr);
    
    stmp = stmp*100;
    
    s=[s stmp];
    l=[l ones(1,length(stmp))*fi];
    
end

s2 = mod([1:length(s)],100);

data = [s;s2;l]';


matwrite(data,'sig.txt');

figure(1);
clf;
plot(data(:,1),'r-');
hold;
plot(data(:,2),'b-');

% %% add some NAN segments

s(length(s)/10:2*length(s)/10) = NaN;
s(5*length(s)/10:6*length(s)/10) = NaN;

data = [s;s2;l]';

matwrite(data,'signan.txt');

figure(2);
clf;
plot(data(:,1),'r-');
hold;
plot(data(:,2),'b-');
