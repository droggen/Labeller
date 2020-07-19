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


#ifndef __SSVIEWLABEL_H
#define __SSVIEWLABEL_H

#include <QWidget>
#include <QFrame>
#include <QSlider>
#include <QVBoxLayout>
#include <QEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QStringList>
#include <vector>
#include "SSViewAbstract.h"
//#include "WidgetLabel.h"
#include "render/NPlot/NPlotWidgetLabel.h"
#include "render/guihelper.h"


/******************************************************************************
   SSViewLabelConfig
*******************************************************************************

   Configuration for SSViewSignal

******************************************************************************/
typedef struct
{
   NPLOTWIDGETLABELCONFIG config;


   std::vector<std::vector<int> *> data;

} SSViewLabelConfig;


/******************************************************************************
   SSViewLabel
*******************************************************************************

   Widget presenting a signal view, with the associated controls (scrollbar, scopes, etc)

******************************************************************************/
class SSViewLabel : public SSViewAbstract
{
   Q_OBJECT
public:
   SSViewLabel(QWidget *parent = 0, int scale=1);
   virtual ~SSViewLabel();

   virtual void setConfig(SSViewLabelConfig c);
   virtual void setTime(int t);
   virtual int getTime();
   virtual void lengthChanged();
   virtual QString name();
   virtual bool hasFocus () const;

   virtual void setNullLabel(std::vector<int> nullabel);


   void setVerticalMarkerBar(bool _markv_enable,int _markv_location,int _markv_color);
   void highlightLabel(bool en, int location);

   virtual void zoomHIn();
   virtual void zoomHOut();
   virtual void zoomHReset();

private:
   // GUI elements
   NPlotWidgetLabel *plotwidget;
   QSliderExt *slider;
   QVBoxLayout *box;
   SSViewLabelConfig config;

private slots:
   virtual void on_Slider_valueChanged(int value);
   virtual void on_View_mousePressed(Qt::MouseButton button, int samplex);
   virtual void on_View_mouseReleased(Qt::MouseButton button, int samplex);
   virtual void on_View_mouseMoved(int samplex);
   virtual void on_View_keyPressed(int key,int samplex);
   virtual void on_View_zoomHInned();
   virtual void on_View_zoomHOuted();
   virtual void on_View_zoomHReseted();



   signals:
   void Leaved();
   void zoomHInned();
   void zoomHOuted();
   void zoomHReseted();
   /*void timeChanged(int value);
   void mousePressed(Qt::MouseButton button,int samplex);
   void mouseMoved(int samplex);*/

protected:

   virtual void leaveEvent(QEvent *event);
  virtual void enterEvent(QEvent *event);

};



#endif // __SSVIEWLABEL_H
