/*
   SynScopeV

   Copyright (C) 2008,2009,2010,2011:
         Daniel Roggen, droggen@gmail.com

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#include "SSViewScope.h"


SSViewScope::SSViewScope(QWidget *parent,int scale) : SSViewAbstract(parent)
{
   plotwidget=0;
   slider=0;
   box=0;

    // Scope
   plotwidget = new NPlotWidgetScopeExt(this);
   plotwidget->setMinimumHeight(200);

   plotwidget->setScale(scale);

   // Slider
   slider = new QSliderExt(Qt::Horizontal,0);
   //slider->setFocusPolicy(Qt::WheelFocus);   // To accept focus, and thus receive keyboard events, when: click, tab, mouse wheel.



   // Box
   box = new QVBoxLayout();
   box->addWidget(plotwidget);
   box->addWidget(slider);
   setLayout(box);

   slider->setVisible(false);




   // QT5
   connect(slider,SIGNAL(valueChanged(int)),this,SLOT(on_Slider_valueChanged(int)));
   connect(plotwidget,SIGNAL(panLeft()),slider,SLOT(scrollLeft()));
   connect(plotwidget,SIGNAL(panRight()),slider,SLOT(scrollRight()));
   connect(plotwidget,SIGNAL(mousePressed(Qt::MouseButton,int)),this,SLOT(on_View_mousePressed(Qt::MouseButton,int)));
   connect(plotwidget,SIGNAL(mouseReleased(Qt::MouseButton,int)),this,SLOT(on_View_mouseReleased(Qt::MouseButton,int)));
   connect(plotwidget,SIGNAL(mouseMoved(int)),this,SLOT(on_View_mouseMoved(int)));
   connect(plotwidget,SIGNAL(keyPressed(int,int)), this,SLOT(on_View_keyPressed(int,int)));
   connect(plotwidget,SIGNAL(zoomHInned()), this,SLOT(on_View_zoomHInned()));
   connect(plotwidget,SIGNAL(zoomHOuted()), this,SLOT(on_View_zoomHOuted()));
   connect(plotwidget,SIGNAL(zoomHReseted()), this,SLOT(on_View_zoomHReseted()));


   plotwidget->installEventFilter(this);
   slider->installEventFilter(this);

}

SSViewScope::~SSViewScope()
{
   //printf("SSViewScope::~SSViewScope");
   if(box) delete box;
   if(slider) delete slider;
   if(plotwidget) delete plotwidget;
}



bool SSViewScope::hasFocus() const
{
    return plotwidget->hasFocus() || slider->hasFocus() || SSViewAbstract::hasFocus();
}



void SSViewScope::setConfig(SSViewScopeConfig c)
{
   config = c;

   plotwidget->setData(c.data,c.config.colors);
   //plotwidget->setTitle(config.config.title);     // Calls wrong method
   plotwidget->SetTitle(config.config.title);
   plotwidget->SetAlignment(true);
   plotwidget->SetGridPosition(true);


   if(config.config.yauto)
      plotwidget->SetVAuto();
   else
      plotwidget->SetVRange(config.config.yscalemin,config.config.yscalemax);
   plotwidget->HZoom(config.config.xscale);

   lengthChanged();
}
void SSViewScope::setVerticalMarkerBar(bool _markv_enable,int _markv_location,int _markv_color)
{
    //printf("SSViewScope::setVerticalMarkerBar: %d %d %d\n",(int)_markv_enable,_markv_location,_markv_color);
    plotwidget->SetVerticalMarkerBar(_markv_enable,_markv_location,_markv_color);
    update();
}

void SSViewScope::setTime(int t)
{
   // When the time is set programmatically we must ensure that no slider signal is emitted

   NoSliderEvents=true;
   slider->setSliderPosition(t);
   NoSliderEvents=false;

}

int SSViewScope::getTime()
{
   return slider->sliderPosition();
}

// Call this to indicate that the data length has changed, for instance due to resampling.
// It adapts the slided length accordingly
void SSViewScope::lengthChanged()
{
   slider->setRange(0,config.data[0]->size());
}


void SSViewScope::on_Slider_valueChanged(int value)
{
   plotwidget->setSampleOffset(value);
   update();

   if(!NoSliderEvents)
      emit timeChanged(value);
}

void SSViewScope::on_View_mousePressed(Qt::MouseButton button, int samplex)
{
   emit mousePressed(button,samplex);
}
void SSViewScope::on_View_mouseReleased(Qt::MouseButton button, int samplex)
{
   emit mouseReleased(button,samplex);
}

void SSViewScope::on_View_mouseMoved(int samplex)
{
   emit mouseMoved(samplex);
}
QString SSViewScope::name()
{
   return QString("Scope");
}
/*
void SSViewScope::paintEvent(QPaintEvent *event)
{
   QFrame::paintEvent(event);

   // Draw a frame if we have the focus
   QPainter p(this);
   p.setPen(Qt::white);
   p.drawRect(0,0,width()-1,height()-1);
   if(hasFocus())
   {
      QPen pen;
      pen.setColor(Qt::black);
      pen.setStyle(Qt::DotLine);
      p.setPen(pen);
      p.drawRect(0,0,width()-1,height()-1);
   }


}*/
void SSViewScope::on_View_keyPressed(int key,int samplex)
{
    //printf("SSViewScope::on_View_keyPressed\n");
    emit keyPressed(key,samplex);
}

void SSViewScope::enterEvent(QEvent *event)
{
    (void) event;
    //printf("Enter event - grab keyboard\n");
    plotwidget->grabKeyboard();

}
void SSViewScope::leaveEvent(QEvent *event)
{
    (void) event;
    //printf("Leave event - ungrab keyboard\n");

    plotwidget->releaseKeyboard();

    emit Leaved();
}
void SSViewScope::on_View_zoomHInned()
{
    //printf("SSViewScope::on_View_zoomHInned\n");
    emit zoomHInned();
}
void SSViewScope::on_View_zoomHOuted()
{
    //printf("SSViewScope::on_View_zoomHOuted\n");
    emit zoomHOuted();
}
void SSViewScope::on_View_zoomHReseted()
{
    //printf("SSViewScope::on_View_zoomHReseted\n");
    emit zoomHReseted();
}



void SSViewScope::zoomHIn()
{
    plotwidget->HZoomin();
    update();
}
void SSViewScope::zoomHOut()
{
    plotwidget->HZoomout();
    update();
}
void SSViewScope::zoomHReset()
{
    plotwidget->HZoomReset();
    update();
}

