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


#include "SSViewLabel.h"


SSViewLabel::SSViewLabel(QWidget *parent,int scale) : SSViewAbstract(parent)
{
   plotwidget=0;
   slider=0;
   box=0;

    // plotwidget
   plotwidget = new NPlotWidgetLabel();
   plotwidget->setMinimumHeight(400);

   plotwidget->setScale(scale);

   // Slider
   slider = new QSliderExt(Qt::Horizontal,0);
   slider->setFocusPolicy(Qt::WheelFocus);   // To accept focus, and thus receive keyboard events, when: click, tab, mouse wheel.

   // Box
   box = new QVBoxLayout();
   box->addWidget(plotwidget);
   box->addWidget(slider);
   setLayout(box);





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

SSViewLabel::~SSViewLabel()
{
   //printf("SSViewLabel::~SSViewLabel");
   if(box) delete box;
   if(slider) delete slider;
   if(plotwidget) delete plotwidget;
}

bool SSViewLabel::hasFocus() const
{
   return plotwidget->hasFocus() || slider->hasFocus() || SSViewAbstract::hasFocus();
}




void SSViewLabel::setConfig(SSViewLabelConfig c)
{
   config = c;

   plotwidget->setData(config.data,config.config.colors);

   plotwidget->setTitle(config.config.title);
   plotwidget->setSubtitle(config.config.subtitle);

   plotwidget->HZoom(config.config.xscale);





   lengthChanged();
}

void SSViewLabel::setNullLabel(std::vector<int> nullabel)
{
    plotwidget->setNullLabel(nullabel);
    // Must trigger a repaint
    repaint();
}

void SSViewLabel::setTime(int t)
{
   // When the time is set programmatically we must ensure that no slider signal is emitted

   NoSliderEvents=true;
   slider->setSliderPosition(t);
   NoSliderEvents=false;

}

int SSViewLabel::getTime()
{
   return slider->sliderPosition();
}

// Call this to indicate that the data length has changed, for instance due to resampling.
// It adapts the slided length accordingly
void SSViewLabel::lengthChanged()
{
   slider->setRange(0,config.data[0]->size());
}

void SSViewLabel::on_Slider_valueChanged(int value)
{
   plotwidget->setSampleOffset(value);
   update();

   if(!NoSliderEvents)
      emit timeChanged(value);
}

void SSViewLabel::on_View_mousePressed(Qt::MouseButton button, int samplex)
{
   emit mousePressed(button,samplex);
}
void SSViewLabel::on_View_mouseReleased(Qt::MouseButton button, int samplex)
{
   emit mouseReleased(button,samplex);
}

void SSViewLabel::on_View_mouseMoved(int samplex)
{
   emit mouseMoved(samplex);
}
QString SSViewLabel::name()
{
   return QString("Label");
}
void SSViewLabel::on_View_keyPressed(int key, int samplex)
{
    printf("SSViewLabeL::on_View_keyPressed\n");
    emit keyPressed(key,samplex);
}


void SSViewLabel::enterEvent(QEvent *event)
{
    (void) event;
    //printf("enter event - grab\n");
    //plotwidget->grabKeyboard();

}
void SSViewLabel::leaveEvent(QEvent *event)
{
    (void) event;
    //printf("leave event - ungrab\n");
    //plotwidget->releaseKeyboard();


    emit Leaved();
}

void SSViewLabel::setVerticalMarkerBar(bool _markv_enable,int _markv_location,int _markv_color)
{
    plotwidget->SetVerticalMarkerBar(_markv_enable,_markv_location,_markv_color);
    update();
}
void SSViewLabel::highlightLabel(bool en,int location)
{
    plotwidget->SetHighlightLabel(en,location);
    update();
}
void SSViewLabel::on_View_zoomHInned()
{
    //printf("SSViewLabel::on_View_zoomHInned\n");
    emit zoomHInned();
}
void SSViewLabel::on_View_zoomHOuted()
{
    //printf("SSViewLabel::on_View_zoomHOuted\n");
    emit zoomHOuted();
}
void SSViewLabel::on_View_zoomHReseted()
{
    //printf("SSViewLabel::on_View_zoomHReseted\n");
    emit zoomHReseted();
}



void SSViewLabel::zoomHIn()
{
    plotwidget->HZoomin();
    update();
}
void SSViewLabel::zoomHOut()
{
    plotwidget->HZoomout();
    update();
}
void SSViewLabel::zoomHReset()
{
    plotwidget->HZoomReset();
    update();
}
