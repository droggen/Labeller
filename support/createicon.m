
ls = 8;
x=-pi:.0001:pi;
y1 = sin(x);
y2 = cos(x);

figure(1);
clf;
hold on;



hl3= rectangle('Position',[-3,-.7,2.5,1.4],...
        'Curvature',0.2,...,
        'FaceColor',[.2 .2 .2],...
         'LineWidth',2,'LineStyle','-')
set(hl3,'EdgeColor',[.5 .5 .5]);

hl4= rectangle('Position',[1.5,-.7,1.5,1.4],...
    'Curvature',0.2,...
    'FaceColor',[.2 .2 .2],...
         'LineWidth',2,'LineStyle','-')
set(hl4,'EdgeColor',[.5 .5 .5]);





hl1 = line([min(x) max(x)],[0 0]);
set(hl1,'Color',[.1 .1 .1]);
hl2 = plot(x,y1,'r-');
%hl3 = plot(x,y2,'b-');
%hl3=hl2;




set(gcf,'GraphicsSmoothing','off')





axis off;
xlim([min(x) max(x)]);
% Remove the ticks - otherwise the place for the labels is reserved, even
% though they are not shown because of 'axis off'.
set(gca,'xticklabel','')
set(gca,'yticklabel','')

r = 150; 

sizes=[512 64 48 32 24];
lss=[8 2 2 1 1];

for s=1:length(sizes)
    set(hl1,'LineWidth',lss(s));
    set(hl2,'LineWidth',lss(s));
    set(hl3,'LineWidth',lss(s));
    set(hl4,'LineWidth',lss(s));
    


    set(gcf, 'PaperUnits', 'inches', 'PaperPosition', [0 0 sizes(s) sizes(s)]/r);

    
    %bg=[.9 .9 1];
    %bg=[0 1 0];
    bg=[0 0 0];
    %whitebg(bg)
    set(gcf,'Color',bg)

    set(gcf,'InvertHardcopy','off')
    
    %set(gcf,'color',bg)
    
    %set(gca,'color',[.8 .1 .8])

    fname = ['sine' num2str(sizes(s)) '.png'];
    %print(gcf,'-dpng',sprintf('-r%d',r), fname);
    print(gcf,'-dpng',sprintf('-r%d',r), 'tmp.png');
    cdata = imread('tmp.png');
    imwrite(cdata, fname, 'png', 'BitDepth', 16, 'transparency', bg)   
end


