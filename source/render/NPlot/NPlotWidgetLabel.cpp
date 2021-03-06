#include "NPlotWidgetLabel.h"




NPlotWidgetLabel::NPlotWidgetLabel(QWidget * parent, Qt::WindowFlags f)
   : NPlotWidgetBase(parent,f)
   , ScopeBase(2,2)
{

    offsetsample=sampleoffset;

    SetVRange(0,10);                    // This isn't used, just to initialize the vertical range
    SetAlignment(true);                 // Align to the center

    nulllabels.clear();                 // By default there are no "null" labels and all the label bounding boxes are plotted



    setMinimumHeight(getIdealHeight(1));

    painter.begin(&pixmap);
    textheight = painter.fontMetrics().height();
    painter.end();

    highlight_enable = false;
}

NPlotWidgetLabel::~NPlotWidgetLabel()
{

}

void NPlotWidgetLabel::setData(const std::vector<std::vector<int> *> &_v,const std::vector<unsigned> &_color)
{
    v=_v;
    color=_color;

    setMinimumHeight(getIdealHeight(v.size()));
}
void NPlotWidgetLabel::setSubtitle(QStringList &sub)
{
   subtitle=sub;
}
unsigned NPlotWidgetLabel::getIdealHeight(int n)
{
   // The desired height is
    int dh = textheight+textheight+(textheight+5)*n;
   return dh*scale;
}
void NPlotWidgetLabel::setSampleOffset(int co)
{
    NPlotWidgetBase::setSampleOffset(co);
    offsetsample = co;
}
// Set the labels for which no bounding box should be plotted
void NPlotWidgetLabel::setNullLabel(std::vector<int> nulllabels)
{
    NPlotWidgetLabel::nulllabels = nulllabels;
}


void NPlotWidgetLabel::plot()
{
    unsigned np;
    //printf("PlotLabeL::Plot. sampleoffset: %d\n",sampleoffset);

/*    printf("data.v.size: %d\n",v.size());
    if(v.size())
        printf("data.v[0].size: %d\n",v[0]->size());*/

   if(v.size()==0)		// if no data to plot make sure we plot the scope frames and axis, but not the trace.
      np = 0;
   else
      np = v[0]->size();


   unsigned nsonscreen;	// number of samples on screen
   if(spp_s==1)
      nsonscreen=w*spp_m;
   else
      nsonscreen=w/spp_m+2;	// round up even if goes out of window




   refsample = offsetsample;
   leftsample = refsample - nsonscreen/2 - 1;
   rightsample = refsample + nsonscreen/2 + 1;

   if(leftsample<0)
      leftsample=0;
   // After setting the refsample,
   // Make sure the rightsample is not larger than the data we have. We must make this AFTER setting refsample (refsample can be larger than the sample we have)
   if(rightsample>np-1)
      rightsample=np-1;

   //printf("leftsample: %d. rightsample: %d. refsample: %d\n",leftsample,rightsample,refsample);

   // Clear the image
   painter.begin(&pixmap);
   painter.fillRect(0,0,pixmap.size().width(),pixmap.size().height(),QColor(0,0,0));

   // Draw the axis
   DrawHGrid(0xFFFFFF,0x3f3f3f,0x7f7f7f,0x7f7f7f);
   // Draw title
   if(title.length())
   {
      painter.setPen(Qt::white);
      painter.drawText(5,textheight,QString(title.c_str()));
   }

   // Draw vertical lines
   QSize s = pixmap.size();
   painter.setPen(Qt::white);
   painter.drawLine(s.width()/2-1,0,s.width()/2-1,s.height()-1);
   painter.drawLine(s.width()/2+1,0,s.width()/2+1,s.height()-1);




   // Plot the labels
   int lastvalue,lastsample;
   // Iterate all the samples
   for(unsigned j=0;j<v.size();j++)
   {
      int y1,y2,yt;
      y1 = 1+textheight+(textheight+5)*j;
      y2 = y1+textheight+2;
      yt = y2-1;
      painter.setPen(QColor((color[j]>>16)&0xff,(color[j]>>8)&0xff,color[j]&0xff));


      // Set the label title.
      if(j<subtitle.size())
         painter.drawText(5,yt,subtitle[j]);

      for(int sample = leftsample;sample<=rightsample;sample++)
      {
         int x1,x2;

         x1 = s2x(lastsample);
         x2 = s2x(sample);



         if(sample==leftsample)           // Initialization upon first call
         {
            lastvalue = v[j]->operator[](sample);
            lastsample=sample;
            painter.drawLine(s2x(sample),y1,s2x(sample),y2);

         }
         else
         {
            // If the current sample is different from the past one OR we're at the last sample, then plot a line
            if(v[j]->operator[](sample) != lastvalue || sample==rightsample)
            {

               char s[256];
               //sprintf(s,"%d: %d",lastsample,v[j]->operator[](lastsample));
                int label = v[j]->operator[](lastsample);
               sprintf(s,"%d",label);

               // Check if the label bounding box must be plotted
               bool plotboundingbox = (std::find(nulllabels.begin(), nulllabels.end(), label) == nulllabels.end());

                bool highlight=false;
                if(highlight_enable)
                {
                    // Check if highlighting necessary
                    if(highlight_location>=lastsample && highlight_location<=sample)
                        highlight=true;

                }
                /*if(highlight)
                    painter.setPen(QColor(0x00,0xff,0x00));
                else
                    painter.setPen(QColor((color[j]>>16)&0xff,(color[j]>>8)&0xff,color[j]&0xff));*/

               QRect br = painter.fontMetrics().boundingRect(s);
               //printf("br: %d,%d. %dx%d. %dx%d\n",br.x(),br.y(),br.width(),br.height(),painter.fontMetrics().width(s),painter.fontMetrics().height());



               painter.drawLine(x2,y1,x2,y2);
               // Plot the horizontal lines
               // TODO: modify not to plot the "null" class.
               if(plotboundingbox)
               {
                   painter.drawLine(x1,y1,x2,y1);
                   painter.drawLine(x1,y2,x2,y2);
                   if(highlight)
                   {
                       for(int b=1;b<3;b++)
                       {
                           painter.drawLine(x1,y1-b,x2,y1-b);
                           painter.drawLine(x1,y2+b,x2,y2+b);
                       }
                   }
               }

               int tw = painter.fontMetrics().width(s);

               // Draw text only if enough space (10% margin)
               if(tw<(x2-x1)*9/10)
                  painter.drawText((x1+x2-tw)/2,yt,s);

               // Keep the last vertical boundary
               lastvalue = v[j]->operator[](sample);
               lastsample = sample;
            }
         }
      }
   }

   // Draw vertical marker
   if(markv_enable)
   {
       // Check if marker is in visible area
        if(markv_location>leftsample+1 && markv_location<rightsample-1)
        {
            painter.setPen(QColor((markv_color>>16)&0xff,(markv_color>>8)&0xff,(markv_color>>0)&0xff));
           // Draw only if not at the extreme left/right
           int x=s2x(markv_location);

            painter.drawLine(x-1,0,x-1,h-1);
            painter.drawLine(x+1,0,x+1,h-1);
        }
   }

   painter.end();

    NPlotWidgetBase::plot();
}
void NPlotWidgetLabel::resizeEvent(QResizeEvent *event)
{
   //printf("NPlotWidgetLabel::resizeEvent\n");
   //ScopeBase::SetSize(event->size().width(),event->size().height());
   //NPlotWidgetBase::resizeEvent(event);

    //ScopeBase::SetSize(event->size().width()/scale,event->size().height()/scale);
    //ScopeBase::SetSize(event->size().width()/scale,event->size().height());
    ScopeBase::SetSize(event->size().width()/scale,event->size().height()/scale);

    // Create a temporary event to fake the scaled size
    //QSize s(event->size().width()/scale,event->size().height());
    QSize s(event->size().width()/scale,event->size().height()/scale);
    QResizeEvent event2(s,event->oldSize());

    //NPlotWidgetBase::resizeEvent(event);           // Original
    NPlotWidgetBase::resizeEvent(&event2);           // With scale

   event->accept();

}
void NPlotWidgetLabel::DrawHGrid(unsigned fcolor_axis,unsigned fcolor_minor,unsigned fcolor_major,unsigned fcolor_text)
{
   // Draw
   int factor,power;
   int xs = XsTickSpacing(factor,power);

   // Original (right-align only) version:...
   int i=0;

   for(i=leftsample/xs-1;i<rightsample/xs+1; i++)
   {
      int posx = s2x(i*xs);
      if(posx<0 || posx>=w)
         continue;

      //printf("XsTickSpacing at sample i: %d i*xs: %d. Unit at pos: %d\n",i,i*xs,s2x(i*xs));
      // Get the color
      unsigned color = fcolor_minor;
      if(((i*factor)%10)==0)
         color=fcolor_major;
      if(i==0)
         color=fcolor_axis;

      painter.setPen(QColor((color>>16)&0xff,(color>>8)&0xff,color&0xff));
      painter.drawLine(s2x(i*xs),0,s2x(i*xs),pixmap.height()-1);




      char s[256];
      //sprintf(s,"%d",-i*xs);
      //if(gridcenter==false)
//         itoa(-i*xs,s,10);
  //    else
         //itoa(i*xs,s,10);
      sprintf(s,"%d",i*xs);
      //if(strlen(s)>5)
         //sprintf(s,"%dE%d",-factor*i,power);
      int sx = painter.fontMetrics().width(s);
      int xpos = s2x(i*xs)-sx/2;
      if(xpos+sx>=w) xpos=w-sx-1;
      if(xpos<0) xpos=0;
      int ypos=pixmap.height()-2;


      painter.drawText(xpos,ypos,s);

   }
}





void NPlotWidgetLabel::wheelEvent (QWheelEvent * event)
{
   QWidget::wheelEvent(event);
    if(event->delta()>0)
    {
            //if(event->modifiers()==Qt::ShiftModifier)
              //      VZoomin();
            //if(event->modifiers()==Qt::NoModifier)
            HZoomin();
            emit zoomHInned();
    }
    else
    {
            //if(event->modifiers()==Qt::ShiftModifier)
              //      VZoomout();
            //if(event->modifiers()==Qt::NoModifier)
            HZoomout();
            emit zoomHOuted();
    }
    event->accept();
    repaint();

}

void NPlotWidgetLabel::keyPressEvent(QKeyEvent * event)
{
   /*if(event->key()==Qt::Key_Up)
   {
      PanUp();
   }
   if(event->key()==Qt::Key_Down)
   {
      PanDown();
   }*/
   if(event->key()==Qt::Key_Left)
   {
      emit panLeft();
   }
   if(event->key()==Qt::Key_Right)
   {
      emit panRight();
   }
   if(event->key()==Qt::Key_PageUp)
   {
      // Zoom
      //if(event->modifiers()==Qt::NoModifier)
         //VZoomin();
      //if(event->modifiers()==Qt::ShiftModifier)
         HZoomin();
         emit zoomHInned();
   }
   if(event->key()==Qt::Key_PageDown)
   {
      // Zoom
      //if(event->modifiers()==Qt::NoModifier)
         //VZoomout();
      //if(event->modifiers()==Qt::ShiftModifier)
         HZoomout();
         emit zoomHOuted();
   }
   if(event->key()==Qt::Key_Return)
   {
      //if(event->modifiers()==Qt::NoModifier)
        // SetVAuto();
      //if(event->modifiers()==Qt::ShiftModifier)
         HZoomReset();
         emit zoomHReseted();
   }

   QPoint p = mapFromGlobal(QCursor::pos());
   int sx = x2s(p.x());
    //printf("mouse coord?: %d %d -> %d\n",p.x(),p.y(),sx);
   emit keyPressed(event->key(),sx);

   event->accept();
   repaint();

}


void NPlotWidgetLabel::SetVerticalMarkerBar(bool _markv_enable,int _markv_location,int _markv_color)
{
    markv_enable=_markv_enable;
    markv_location=_markv_location;
    markv_color=_markv_color;
}


/******************************************************************************
   Signals & Slots
*******************************************************************************
******************************************************************************/
void NPlotWidgetLabel::mouseMoveEvent ( QMouseEvent * event )
{
   emit mouseMoved(x2s(event->x()/scale));
   event->accept();
}


void NPlotWidgetLabel::mousePressEvent ( QMouseEvent * event )
{
   // signal / slot
   emit mousePressed(event->button(),x2s(event->x()/scale));

   //printf("mouse pressed: %d %d %d\n",event->x(),event->y(),event->button());
   //printf("Converstion of pix -> sample %d -> %d\n",event->x(),x2s(event->x()));
    switch(event->button())
    {
            case Qt::MidButton:
                    switch(event->modifiers())
                    {
                            case Qt::ShiftModifier:
                                    SetVAuto();
                                    break;
                            case Qt::NoModifier:
                                    HZoomReset();
                                    emit zoomHReseted();
                                    break;
                            default:
                                    event->ignore();
                                    return;
                    }
                    break;
            default:
                    event->ignore();
                    return;
    }
    event->accept();
    repaint();
}
void NPlotWidgetLabel::mouseReleaseEvent ( QMouseEvent * event )
{
   // signal / slot
   emit mouseReleased(event->button(),x2s(event->x()/scale));

    //printf("NPlotWidgetLabel::mouseReleaseEvent\n");

   /*event->ignore();
   repaint();*/
}
void NPlotWidgetLabel::SetHighlightLabel(bool enable,int location)
{
    highlight_enable = enable;
    highlight_location = location;
}
