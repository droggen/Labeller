#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QScrollBar>
#include <QSettings>
#include <QScreen>
#include <QTextStream>
#include <Qt>
#include <cassert>
#include "cio.h"
#include "harlib/fastmatread.h"
#include "render/SSView/SSViewScope.h"
#include <vector>
#include "dataset.h"
#include <dialogenterlabel.h>
#include "helpwindow.h"

/*
 * TODOBUG
 * - When adding a label prior to "confirm"; then the label box is populated with all instances; but when undoing all instances are removed.
 *
 *
 *
*/
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , terminal(100)
{
    ui->setupUi(this);

    scopeviewsClear();
    labelview=0;

    clearAllState();
    uiSetToNoDataset();

#ifdef DEVELMODE
    ConsoleInit();
#endif

    QPixmap curleft_pixmap(":/img/cursor-toleft2.png");
    QPixmap curright_pixmap(":/img/cursor-toright2.png");

    curleft = QCursor(curleft_pixmap);
    curright = QCursor(curright_pixmap);


    // Heuristic to find a proper scale
    if(QApplication::primaryScreen()->physicalDotsPerInchX()>400)
        scale = 4;
    else
        if(QApplication::primaryScreen()->physicalDotsPerInchX()>200)
            scale=2;
        else
            scale=1;
    //scale=2;      // Tryout scale


}

MainWindow::~MainWindow()
{
    delete ui;
}
/*
 * Clear all the data structures (but not the views) and restore values to defaults.
 * Views must be erased before this is called.
*/
void MainWindow::clearAllState()
{
    labelchannel=0;
    proximitytolerance = 20;
    isgrabbing=false;                    // Indicate if an instance is being grabbed
    ispainting=false;                    // Indicate if a new instance is being "painted"
    islabelconfirmed=false;
    instanceedited.instance=-1;    // Indicates which instance is grabbed

    ignoreTableSelectN=0;

    nulllabels.clear();
    nulllabels.push_back(0);

    scopedefinitions.clear();


    // Clear all dataset related data structures.
    dataset_reset(dataset);     // Initialise the dataset structure
    clearLabelStructures();
}
/*
 * Deactivate all the UI elements when no dataset is loaded
*/
void MainWindow::uiSetToNoDataset()
{
    // Remove all the views
    viewsClear();

    // Clear the terminal
    terminalClear();


    // UI when waiting for dataset to be loaded - deactivate most of the UI elements.
    ui->uipb_apply->setEnabled(false);
    ui->uipb_updatelabeltable->setEnabled(false);
    ui->uile_formatstring->setEnabled(false);
    ui->uile_null_label->setEnabled(false);
    ui->uisb_labelchannel->setEnabled(false);
    // Same with menus
    ui->action_Add_column->setEnabled(false);
    ui->action_Export_dataset->setEnabled(false);
    ui->action_Undo_label_change->setEnabled(false);
    ui->action_Save_project->setEnabled(false);

    // Set some of the parameters
    ui->uile_formatstring->setText(""); formatstring="";
    ui->uile_null_label->setText(nulllabelVecToString(nulllabels));

    // Set label channel to 0
    ui->uisb_labelchannel->setValue(labelchannel);

    // Update info about dataset in UI
    ui->uil_data_filename->setText("");
    ui->uil_data_foldername->setText("");
    ui->uil_data_size->setText("");

}
/*
 * Activate all the UI elements when a dataset is loaded
*/
int MainWindow::uiSetToDataset()
{
    // Can transition to this state from Idle or from Confirmed.

    // UI when waiting for dataset to be loaded - activate most of the UI elements.
    ui->uipb_apply->setEnabled(true);
    ui->uipb_updatelabeltable->setEnabled(true);
    ui->uile_formatstring->setEnabled(true);
    ui->uile_null_label->setEnabled(true);
    ui->uisb_labelchannel->setEnabled(true);
    // Same with menus
    ui->action_Add_column->setEnabled(true);
    ui->action_Export_dataset->setEnabled(true);
    ui->action_Undo_label_change->setEnabled(true);
    ui->action_Save_project->setEnabled(true);

    // Update info about dataset in UI
    ui->uil_data_filename->setText(QString::fromStdString(dataset.filename_nopath));
    ui->uil_data_foldername->setText(QString::fromStdString(dataset.filename_path));
    ui->uil_data_size->setText(QString("%1 x %2").arg(dataset.sy).arg(dataset.sx));

    return 0;
}
void MainWindow::uiPrepareDefaultFromDataset()
{
    // Setup UI with default settings

    // Clear the terminal whenever we load a data file or project
    //terminalClear();

    // Create a format string
    formatstring = createDefaultFormatStringFromDataset(QString::fromStdString(dataset.filename_nopath));
    ui->uile_formatstring->setText(formatstring);
    ScopesDefinition sd;
    int rv = checkFormatString(sd);
    assert(rv==0);          // This must succeed always
    scopedefinitions=sd;

    // Create the null string
    ui->uile_null_label->setText(nulllabelVecToString(nulllabels));

    // Limit the label spin box to the range of channels
    ui->uisb_labelchannel->setMaximum(dataset.sx-1);
}
void MainWindow::on_action_Load_triggered()
{
    //loadPromptSync();
    loadPromptAsync();
}
void MainWindow::loadPromptSync()
{
    // Pop up window
    QString fileName = QFileDialog::getOpenFileName(this, "Load data",QString(),"Data (*.dat *.txt);;All files (*)");

    // No file: return
    if(fileName.isNull())
        return;

    // Clear all dataset and state variables
    clearAllState();
    // Set the UI to no dataset (deactivate the UI elements)
    uiSetToNoDataset();

    // Try load dataset; failure: return
    if(loadDataset(fileName)!=0)
    {
        uiSetToNoDataset();
        return;
    }

    // Setup UI according to loaded dataset
    uiPrepareDefaultFromDataset();
    viewsCreate();
    uiSetToDataset();

}
void MainWindow::loadPromptAsync()
{
    // Asynchronous loading for webassembly

    // Clear all dataset and state variables
    clearAllState();
    // Set the UI to no dataset (deactivate the UI elements)
    uiSetToNoDataset();


    auto fileContentReady = [this](const QString &fileName, const QByteArray &fileContent) {
        printf("fileContentReady: this: %p\n",this);
        this->loadAsynCompleted(fileName,fileContent);
    };
    printf("Prior to call: this: %p\n",this);
    QFileDialog::getOpenFileContent("Data (*.dat *.txt);;All files (*)", fileContentReady);

    // Now the data is asynchronously loading
    //uiSetToLoadingDataset();
}


void MainWindow::toto(const QString &fileName, const QByteArray &fileContent)
{
    printf("toto!\n");
    if(fileName.isEmpty())
    {
        printf("file not specified\n");
    }
    else
    {
        printf("File: %s. Size: %d\n",fileName.toStdString().c_str(),fileContent.size());
    }
}

void MainWindow::loadAsynCompleted(const QString &fileName, const QByteArray &fileContent)
{
    printf("loadAsynCompleted\n");
    if(fileName.isEmpty())
    {
        printf("No file selected\n");
        return;
    }

    printf("File: %s. Size: %d\n",fileName.toStdString().c_str(),fileContent.size());
    QFileInfo fileInfo(fileName);
    QString filename_nopath = fileInfo.fileName();
    QString filename_path = fileInfo.path();
    printf("File no path: %s\n",filename_nopath.toStdString().c_str());
    printf("File path: %s\n",filename_path.toStdString().c_str());


    dataset.filename = fileName.toStdString();
    dataset.filename_nopath = fileName.toStdString();
    dataset.filename_path = "<No path>";

    int rv;
    QByteArray fc = fileContent;
    dataset.data = LoadSignalFromByteArray(fc,rv, true);

    if(rv!=0)
    {
        QMessageBox::critical(this,"Error", "Error loading file");
        uiSetToNoDataset();
        return;
    }

    dataset.sx = (unsigned)dataset.data.size();
    dataset.sy = (unsigned)dataset.data[0].size();

    terminalPrint(QString("Loaded: %1\n").arg(QString::fromStdString(dataset.filename_nopath)));
    printf("Loading: %s\n",dataset.filename.c_str());
    terminalPrint(QString("Dataset is %1 x %2\n").arg(dataset.sy).arg(dataset.sx));

    // Setup UI according to loaded dataset
    uiPrepareDefaultFromDataset();
    viewsCreate();
    uiSetToDataset();

}

bool MainWindow::loadDataset(QString filename)
{
    // Get the name of the file only
    QFileInfo fileInfo(filename);
    QString filename_nopath = fileInfo.fileName();
    QString filename_path = fileInfo.path();

    dataset.filename = filename.toStdString();
    dataset.filename_nopath = filename_nopath.toStdString();
    dataset.filename_path = filename_path.toStdString();

    int rv;
    dataset.data = LoadSignalFile(filename.toStdString(),rv,true);

    if(rv!=0)
    {
        QMessageBox::critical(this,"Error", "Error loading file");
        return -1;
    }

    dataset.sx = dataset.data.size();
    dataset.sy = dataset.data[0].size();

    terminalPrint(QString("Loaded: %1\n").arg(QString::fromStdString(dataset.filename_nopath)));
    printf("Loading: %s\n",dataset.filename.c_str());
    terminalPrint(QString("Dataset is %1 x %2\n").arg(dataset.sy).arg(dataset.sx));

    return 0;
}


/* Create a default format string with the filename as title, and all the columns selected.
 *
*/
QString MainWindow::createDefaultFormatStringFromDataset(QString filename)
{
    QString format;
    format.sprintf("<%s;",filename.toStdString().c_str());
    for(unsigned i=0;i<dataset.sx;i++)
    {
        format+=QString("%1 ").arg(i);
    }
    format+=QString(";xscale=1;yscale=auto>");
    return format;
}
void MainWindow::scopesCreateFromScopeDefinitions()
{
    // The format string has been verified previously - add the scopes.

    // Add N scopes
    scopeviewsAddN(scopedefinitions.size());

    // For each of the N scopes, set their parameters
    for(unsigned si=0;si<scopedefinitions.size();si++)
    {
        // Convert the scopedef structure into the data needed by SSViewScope (this could be much more elegant)

        SSViewScopeConfig scopeviewconfig;                  // Configuration for the scope - does not include the "traces" member, but data points
        ScopeDefinition scopedef = scopedefinitions[si];    // Parsed configuration - includes the trace member

        scopeviewconfig.config.title=scopedef.title;
        scopeviewconfig.config.colors=scopedef.colors;
        scopeviewconfig.config.xscale=scopedef.xscale;
        scopeviewconfig.config.yauto=scopedef.yauto;
        if(scopedef.yauto==false)
        {
            scopeviewconfig.config.yscalemin=scopedef.yscale[0];
            scopeviewconfig.config.yscalemax=scopedef.yscale[1];
        }

        // Iterate all the traces and set pointer to corresponding data
        for(unsigned i=0;i<scopedef.traces.size();i++)
        {
            assert(scopedef.traces[i]<dataset.sx);  // This was tested prior so should be ok

            scopeviewconfig.data.push_back(&dataset.data[scopedef.traces[i]]);
        }

        scopeviews[si]->setConfig(scopeviewconfig);
        scopeviews[si]->update();
    }
}
void MainWindow::addViewLabel()
{
    // Must be called without any view allocated beforehand
    assert(labelview==0);

    labelview = new SSViewLabel(this,scale);

    ui->uif_plots->layout()->addWidget(labelview);

    SSViewLabelConfig sc;
    viewlabelconfig = sc;
    viewlabelconfig.config.title="Some title";
    viewlabelconfig.config.yauto=true;
    viewlabelconfig.config.xscale=1;
    QStringList sl;
    sl.append("Label");
    viewlabelconfig.config.subtitle=sl;
    // Only one label track; get the label to display
    assert(labelchannel>=0); assert(labelchannel<(int)dataset.sx);
    viewlabelconfig.data.push_back(&dataset.data[labelchannel]);
    viewlabelconfig.config.colors.push_back(0xffffff);


    labelview->setConfig(viewlabelconfig);

    labelview->setNullLabel(nulllabels);

    connect(labelview,SIGNAL(timeChanged(int)),this,SLOT(on_View_timeChanged(int)));
    connect(labelview,SIGNAL(mousePressed(Qt::MouseButton,int)),this,SLOT(on_View_mousePressed(Qt::MouseButton,int)));
    connect(labelview,SIGNAL(mouseReleased(Qt::MouseButton,int)),this,SLOT(on_View_mouseReleased(Qt::MouseButton,int)));
    connect(labelview,SIGNAL(mouseMoved(int)),this,SLOT(on_View_mouseMoved(int)));
    connect(labelview,SIGNAL(keyPressed(int,int)),this,SLOT(on_View_keyPressed(int,int)));
    connect(labelview,SIGNAL(Leaved()),this,SLOT(on_View_Leaved()));
    connect(labelview,SIGNAL(zoomHInned()), this,SLOT(on_View_zoomHInned()));
    connect(labelview,SIGNAL(zoomHOuted()), this,SLOT(on_View_zoomHOuted()));
    connect(labelview,SIGNAL(zoomHReseted()), this,SLOT(on_View_zoomHReseted()));

}






void MainWindow::printScopedefs(ScopesDefinition sd)
{
    printf("Scope definitions: %llu\n",sd.size());
    for(unsigned i=0;i<sd.size();i++)
    {
        printf("\tScope definition #%d\n",i);
        printf("\t\tTitle: %s\n",sd[i].title.c_str());
        printf("\t\tX scale: %d\n",sd[i].xscale);
        printf("\t\tY auto: %s\n",sd[i].yauto?"Yes":"No");
        printf("\t\tY scale: ");
        for(unsigned t=0;t<sd[i].yscale.size();t++)
            printf("%d ",sd[i].yscale[t]);
        printf("\n");
        printf("\t\tTraces: ");
        for(unsigned t=0;t<sd[i].traces.size();t++)
            printf("%d ",sd[i].traces[t]);
        printf("\n");
        printf("\t\tColors: ");
        for(unsigned t=0;t<sd[i].colors.size();t++)
            printf("%Xh ",sd[i].colors[t]);
        printf("\n");
    }
}
/* Parse the scope format string into scopedefinitions
 *
 * Returns:
 *  0       -   Success
 *  nonzero -   Error
*/
int MainWindow::scopeviewsParseFormat(QString format,ScopesDefinition &scopedefs)
{
    // Parse format string

    int rv = ParseLayoutString(format.toStdString(),scopedefs);

    if(rv!=0)
    {
       printf("Error in layout string\n");
       QMessageBox::information(this, "Invalid display format string","Check help for more information");
       return 1;
    }

    // Print the format strings
    printScopedefs(scopedefs);

    // Check that the traces are within the size of the dataset
    for(unsigned si=0;si<scopedefs.size();si++)
    {
        bool ok = true;
        for(unsigned t=0;t<scopedefs[si].traces.size();t++)
        {
            if(scopedefs[si].traces[t]>=dataset.sx)
                ok=false;
        }
        if(!ok)
        {
            QString str;
            str.sprintf("The traces specified in the scope definition %d do not correspond to available data channels (%d channels)",si,dataset.sx);
            printf("%s\n",str.toStdString().c_str());
            QMessageBox::information(this, "Error", str);
            return 1;
        }
    }

    // All the traces are valid

    // Ensure colors are specified - add them as white if unspecified
    for(unsigned si=0;si<scopedefs.size();si++)
    {
        if(scopedefs[si].colors.size()==0)
        {
            for(unsigned i=0;i<scopedefs[si].traces.size();i++)
                scopedefs[si].colors.push_back(0xffffff);
        }
    }

    return 0;
}




/******************************************************************************
   MainWindow::on_View_timeChanged
*******************************************************************************
   Called when the time in a view (scope) has been changed by the user.
******************************************************************************/
void MainWindow::on_View_timeChanged(int value)
{
    setTime(value);

}
void MainWindow::setTime(int value)
{
    labelview->setTime(value);

    for(unsigned i=0;i<scopeviews.size();i++)
        scopeviews[i]->setTime(value);
}
/**
  \brief Slot handling mouse button press in a scope or video view. Find view and pass to ViewClick
**/
void MainWindow::on_View_mousePressed(Qt::MouseButton button, int samplex)
{
   //printf("view mouse pressed: %d %d\n",button,samplex);

   // Left button: center on click location
   if(button==Qt::LeftButton)
   {
        labelview->setTime(samplex);
        for(unsigned i=0;i<scopeviews.size();i++)
            scopeviews[i]->setTime(samplex);
   }

   if(button==Qt::RightButton)
   {
       if(!islabelconfirmed)
           return;

        // Grab, if within grabbing range
       INSTANCEGRABBED ig = checkProximityInstance(samplex,proximitytolerance);
       instanceedited=ig;
       if(ig.instance==-1)
       {
           ispainting=true;
           // Change cursor
           labelview->setCursor(Qt::SizeHorCursor);
           for(unsigned i=0;i<scopeviews.size();i++)
            scopeviews[i]->setCursor(Qt::SizeHorCursor);
       }
       else
       {
           // Something in grabbing range
           isgrabbing=true;
           printf("Grabbing instance %d from %s\n",ig.instance,ig.isfrom?"left":"right");
       }
   }

}
void MainWindow::on_View_mouseReleased(Qt::MouseButton button, int samplex)
{
   //printf("view mouse released: %d %d\n",button,samplex);
   if(!islabelconfirmed)
       return;

   if(button==Qt::RightButton)
   {
       INSTANCEGRABBED ig = checkProximityInstance(samplex,proximitytolerance);
       // Restore cursor depending on current location
       changeCursorGrabbing(ig);


       if(!isgrabbing && !ispainting)
       {
           // If we weren't grabbing nor painting: don't process further: occurs when labels changed
            return;
       }


        // Release
       if(isgrabbing)
       {
            // Grabbing
           printf("Ungrabbbed instance %d from %s at %d.\n",instanceedited.instance,instanceedited.isfrom?"left":"right",samplex);
           if(instanceedited.instance<0)
               return;

            printf("\tBoundary changes from %d to %d\n",instanceedited.isfrom?labels[instanceedited.instance].from:labels[instanceedited.instance].to,samplex);

           instanceChange(instanceedited,samplex);
       }
       else
       {
           // Painting
           printf("Finished painting from %d to %d.\n",instanceedited.at,samplex);
#if 0
           // Synchronous implementation
           // Query label
           bool ok;
           int v = dialogGetLabelID(ok);
           // If ok, add label
           if(ok)
            instanceAdd(instanceedited.at,samplex,v);
#endif
           // Asynchronous implementation
           // Query label
           std::function<void (bool,int)> callback=[=](bool ok, int v) {
            printf("Callback add label %d %d\n",(int)ok,v);
                if(ok)
                    instanceAdd(instanceedited.at,samplex,v);
           };

           dialogGetLabelID2(callback);



       }
       isgrabbing=false;
       ispainting=false;


   }

}
void MainWindow::on_View_keyPressed(int key, int samplex)
{
    //printf("MainWindow::on_View_keyPressed(QKeyEvent*). key: %d samplex: %d\n",key,samplex);
    if(!islabelconfirmed)
        return;

    // Check if delete key
    if(key == Qt::Key_Delete)
    {
        // Check if there is a label spanning samplex
        for(unsigned i=0;i<labels.size();i++)
        {
            if(samplex>=(int)labels[i].from && samplex<=(int)labels[i].to)
            {
                instanceDelete(i);
                return;
            }

        }
    }


}
void MainWindow::on_View_Leaved()
{
    //printf("leaved\n");

    // Deactivate all the markers and highlighting in the scopes/label view
    labelview->highlightLabel(false,0);
    labelview->setVerticalMarkerBar(false,0,0);
    for(unsigned i=0;i<scopeviews.size();i++)
        scopeviews[i]->setVerticalMarkerBar(false,0,0);

}
/**
  \brief Slot handling mouse move press in a scope or video view.
**/
void MainWindow::on_View_mouseMoved(int samplex)
{
   //printf("View mouse moved: %d. isgrabbing: %d ispainting: %d\n",samplex,(int)isgrabbing,(int)ispainting);

    // If labels are not confirmed, we do not do any cursor changes or display markers
   if(!islabelconfirmed)
       return;

   // Highlight label
   labelview->highlightLabel(true,samplex);

   // Only change the cursor if not in "grabbing" mode.
    INSTANCEGRABBED ig = checkProximityInstance(samplex,proximitytolerance);

    if(!isgrabbing && !ispainting)          // Change cursor only if not grabbing and not painting
        changeCursorGrabbing(ig);

    // Set a vertical marker bar at the location. If grabbed does smooth motion (samplex); if ungrabbed does snap motion (ig.at)
    if(isgrabbing || ispainting)
    {
        for(unsigned i=0;i<scopeviews.size();i++)
            scopeviews[i]->setVerticalMarkerBar(true,samplex,0xff0000);
        labelview->setVerticalMarkerBar(true,samplex,0xff0000);
    }
    else
    {
        for(unsigned i=0;i<scopeviews.size();i++)
            scopeviews[i]->setVerticalMarkerBar(true,ig.at,0xff0000);
        labelview->setVerticalMarkerBar(true,ig.at,0xff0000);
    }
}
void MainWindow::changeCursorGrabbing(INSTANCEGRABBED ig)
{
   //printf("\tLabel: %d. isfrom: %d\n",ig.instance,ig.isfrom);

   if(ig.instance>=0)
   {
        if(ig.isfrom)
        {
            labelview->setCursor(curright);
            for(unsigned i=0;i<scopeviews.size();i++)
                scopeviews[i]->setCursor(curright);
        }
        else
        {
            labelview->setCursor(curleft);
            for(unsigned i=0;i<scopeviews.size();i++)
                scopeviews[i]->setCursor(curleft);
        }
   }
   else
   {
        // TODO: should restore cursor only if it was changed to non-arrow
        labelview->setCursor(QCursor(Qt::ArrowCursor));
        for(unsigned i=0;i<scopeviews.size();i++)
            scopeviews[i]->setCursor(QCursor(Qt::ArrowCursor));
   }
}











void MainWindow::on_uisb_labelchannel_valueChanged(int arg1)
{
    printf("MainWindow::on_uisb_labelchannel_valueChanged: %d\n",arg1);
    setLabelChannel(arg1);
}
void MainWindow::setLabelChannel(int arg1)
{
    printf("setLabelChannel: %d\n",arg1);
    // The label channel has changed -> update structures.
    labelchannel = arg1;

    // Whenever a label changes, we clear the label table.
    clearLabelStructures();

    // This function may be called when no dataset is loaded - in which case return.
    if(dataset.sx==0 || dataset.sy==0)
        return;

    unsigned numinst = datasetCountInstances(dataset,nulllabels,labelchannel);
    terminalPrint(QString("Label channel: %1\n").arg(labelchannel));
    terminalPrint(QString("Number of instances: %1\n").arg(numinst));

    // Update the pointer to the data
    assert(labelchannel<dataset.sx);
    viewlabelconfig.data.clear();
    viewlabelconfig.data.push_back(&dataset.data[labelchannel]);

    if(labelview)
    {
        // labelview may not be created yet
        labelview->setConfig(viewlabelconfig);
        labelview->repaint();
    }
}
void MainWindow::populateLabelTable()
{
    //printf("populateLabelTable\n");
    ignoreTableSelectN=2;           // Must ignore two changes - one clearing the table; one populating it
    //printf("populateLabelTable: calling clearContents\n");
    ui->uitw_labels->clearContents();
    //printf("populateLabelTable: calling setRowCount\n");
    ui->uitw_labels->setRowCount(0);                    // 0 ensures all the widgets are deleted
    ui->uitw_labels->setRowCount(labels.size());        // desired size


    //for(int t=0;t<2;t++)
    //{
      //  printf("t: %d\n",t);
    //printf("populateLabelTable: Creating items\n");
    // try twice

    for(unsigned i = 0; i < labels.size(); i++)
    {
       QTableWidgetItem *newItem;
       newItem = new QTableWidgetItem(QString("%1").arg(labels[i].from));
       //newItem->setFlags(Qt::NoItemFlags);
       //newItem->setFlags(Qt::ItemIsEnabled);
       ui->uitw_labels->setItem(i,0, newItem);
       newItem = new QTableWidgetItem(QString("%1").arg(labels[i].to));
       //newItem->setFlags(Qt::NoItemFlags);
       ui->uitw_labels->setItem(i,1, newItem);
       newItem = new QTableWidgetItem(QString("%1").arg(labels[i].label));
       //newItem->setFlags(Qt::NoItemFlags);
       ui->uitw_labels->setItem(i,2, newItem);

    }
//}
    //printf("populateLabelTable: setSectionResizeMode\n");
    ui->uitw_labels->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    //printf("populateLabelTable: setStretchLastSection\n");
    ui->uitw_labels->horizontalHeader()->setStretchLastSection(true);

    // Select first row, if available

    //printf("populateLabelTable: selectRow\n");
    ui->uitw_labels->selectRow(0);

}


int MainWindow::dialogGetLabelID(bool &ok,QString expl,QString title)
{
    int v=0;
    ok=false;
    // Spawn dialog box
    DialogEnterLabel dialog(this);
    if(!expl.isEmpty())
        dialog.setExplanation(expl,title);
    int ret = dialog.exec();
    if(ret == QDialog::Accepted)
    {
        v = dialog.getLabel(ok);
    }
    return v;

}
void MainWindow::dialogGetLabelID2(std::function<void (bool,int)> callback,QString expl, QString title)
{
    DialogEnterLabel *dialog = new DialogEnterLabel(this);
    dialog->setExplanation(expl,title);
    connect(dialog,&DialogEnterLabel::dataEntered,
                [=](bool ok, int v) {
                    printf("callback: ok: %d v: %d\n",(int)ok,v);
                    callback(ok,v);
                }
            );
    dialog->open();


}
void MainWindow::updateLabels()
{
    createInstanceStructureFromDataset();
    populateLabelTable();
}
void MainWindow::createInstanceStructureFromDataset()
{
    //printf("createInstanceStructureFromDataset\n");

    // Convers the label channel into a label data structure
    assert(dataset.sx>1); assert(dataset.sy>1);
    assert(labelchannel<dataset.sx);




    labels.clear();
    LABEL l;

    bool doinit=true;
    for(unsigned i=1;i<dataset.sy;i++)   // Start at 1; plays with i-1 in doinit
    {
        if(doinit)
        {
            l.from = i-1;
            l.label = dataset.data[labelchannel][i-1];
            doinit=false;
        }
        if(dataset.data[labelchannel][i] != l.label)
        {
            // label has changed -> add start-end
            l.to = i-1;
            // Only add the label if it is not a NULL label
            if( std::find(nulllabels.begin(), nulllabels.end(), l.label) == nulllabels.end() )
            {
                //printf("Adding %d-%d %d in main loop\n",l.from,l.to,l.label);
                labels.push_back(l);
            }
            // Reinit for next label
            doinit=true;
        }
    }
    // If doinit is still set it means a label changed on the last sample. We lose the last sample.
    if(!doinit)
    {
        l.to = dataset.sy-1;
        // Only add the label if it is not a NULL label
        if( std::find(nulllabels.begin(), nulllabels.end(), l.label) == nulllabels.end() )
        {
            //printf("Adding %d-%d %d at end\n",l.from,l.to,l.label);
            labels.push_back(l);
        }
    }

    //printLabelStructure(labels);

}

void MainWindow::printLabelStructure(std::vector<LABEL> labels)
{
    printf("printLabelStructure: number of instances: %d\n",(int)labels.size());
    //return;
    for(unsigned i=0;i<labels.size();i++)
    {
        printf("%d: %d -> %d @ %d\n",i,labels[i].from,labels[i].to,labels[i].label);
    }
}
// Check the format string and prompt the user to create a default one if needed.
int MainWindow::checkFormatString(ScopesDefinition &scopesdef)
{
    QString fstr = ui->uile_formatstring->text(); formatstring=fstr;
    while(scopeviewsParseFormat(fstr,scopesdef)!=0)
    {
        // Scopes definition is invalid.
        if( QMessageBox::question(this,"Format string",
                                    "The format string is invalid.\nCreate a default format string to visualise all the channels?",
                                    QMessageBox::Yes|QMessageBox::No)
                                  == QMessageBox::No)
        {
            // User does not want to create a default format string and it is invalid: return error
            return 1;
        }
        else
        {
            fstr = createDefaultFormatStringFromDataset(QString::fromStdString(dataset.filename_nopath));
            ui->uile_formatstring->setText(fstr); formatstring=fstr;
            // The while loop will ensure this is parsed into scopesdef
        }
    }
    return 0;
}
// Check the format string and prompt the user to create a default one if needed.
int MainWindow::checkNullLabels(std::vector<int> &nl)
{
    QString nlstr = ui->uile_null_label->text();
    while(nulllabelStringToVec(nlstr,nl))
    {
        // Null string invalid
        if( QMessageBox::question(this,"Null labels",
                                    "The null labels are invalid. Possibly non-numerical values were specified.\nSets the null label to zero?",
                                    QMessageBox::Yes|QMessageBox::No)
                                  == QMessageBox::No)
        {
            // User does not want to create a default format string and it is invalid: return error
            return 1;
        }
        else
        {
            nlstr="0";
            ui->uile_null_label->setText(nlstr);
        }
    }
    return 0;
}


void MainWindow::on_uipb_apply_clicked()
{
    // Get the data which could have been modified by the user

    // Check format string: if invalid and default not accepted, return.
    ScopesDefinition scopesdef;
    if(checkFormatString(scopesdef))
        return;

    // Check null string: if invalid and default not accepted, return.
    std::vector<int> nl;
    if(checkNullLabels(nl))
        return;

    // All ok so far; update state.
    scopedefinitions = scopesdef;
    nulllabels = nl;


    // Clear the label table (TODO: should do this only if the null labels changed)
    clearLabelStructures();

    // Create the scope and label views
    viewsCreate();
}
void MainWindow::clearLabelStructures()
{
    islabelconfirmed=false;
    labels.clear();
    labels_back.clear();
    populateLabelTable();
}
void MainWindow::viewsCreate()
{
    // Apply all the settings: show scope; show label; update label null class

    // Erase all views; then check if there's a dataset loaded
    viewsClear();

    if(dataset.sx==0 || dataset.sy==0)
    {
        QMessageBox::critical(this,"Error", "No dataset loaded. Load a dataset first.");
        return;
    }


    // Create the scopes from the format string
    scopesCreateFromScopeDefinitions();
    addViewLabel();
}
void MainWindow::terminalPrint(QString s)
{
    terminal.addBytes(s.toUtf8());
    QString str=terminal.getText();
    ui->uipte_labelinfo->setPlainText(str);
    ui->uipte_labelinfo->verticalScrollBar()->setValue(ui->uipte_labelinfo->verticalScrollBar()->maximum());
}
void MainWindow::terminalClear()
{
    terminal.clear();
    QString str=terminal.getText();
    ui->uipte_labelinfo->setPlainText(str);
    ui->uipte_labelinfo->verticalScrollBar()->setValue(ui->uipte_labelinfo->verticalScrollBar()->maximum());
}

void MainWindow::on_uitw_labels_itemSelectionChanged()
{
    int row = ui->uitw_labels->currentRow();
    //printf("on_uitw_labels_itemSelectionChanged: row: %d\n",row);

    if(ignoreTableSelectN)
    {
        // Ignore a table change - this is useful when the table is repopulated when changing a label, to avoid changing the scope location
        ignoreTableSelectN--;
        return;
    }



    // Update the location based on the selection

    int from = labels[row].from;
    if(labelview)
        labelview->setTime(from);
    for(unsigned i=0;i<scopeviews.size();i++)
        scopeviews[i]->setTime(from);

}




void MainWindow::on_uipb_updatelabeltable_clicked()
{
    printf("on_uipb_updatelabeltable_clicked\n");
    // Backup labels don't persist when a channel is changed
    labels_back.clear();


    printf("calling createInstanceStructureFromDataset\n");
    createInstanceStructureFromDataset();

    // Sanity check
    unsigned lim = 1000;
    if(labels.size()>lim)
    {
        if( QMessageBox::question(this,"Confirm",
                                  QString("There are more than %1 instances with the selected label channel.\nDo you really want to populate the label table?").arg(lim),
                                  QMessageBox::Yes|QMessageBox::No) == QMessageBox::No)
        {
            labels.clear();
            return;
        }
    }

    islabelconfirmed=true;

    printf("calling populateLabelTable\n");
    populateLabelTable();
    printf("calling printLabelStat\n");
    printLabelStat();
}
void MainWindow::printLabelStat()
{
    terminalPrint("Label statistics\n");
    terminalPrint(QString("Number of instances: %1\n").arg(labels.size()));
    // must get number of instances per class
    std::map<int,int> label_numinst;
    for(unsigned i=0;i<labels.size();i++)
    {
        label_numinst[labels[i].label]=label_numinst[labels[i].label]+1;
    }
    terminalPrint(QString("Number of unique labels: %1\n").arg(label_numinst.size()));
    for(auto i=label_numinst.begin();i!=label_numinst.end();i++)
    {
        terminalPrint(QString("Label %1: %2 instances\n").arg(i->first).arg(i->second));
    }

}
/*
 * Check if the mouse at position is close to a label boundary within tolerance.
 * Returns: -1 if not close to a label.
 * Returns: >=0 the instance number in the labels structure.
*/
INSTANCEGRABBED MainWindow::checkProximityInstance(int position,int tolerance)
{
    INSTANCEGRABBED ig;



    ig.at = position;
    ig.instance=-1;

    // TODO: The labels structure is sorted - this can be used to speed up search
    // BUG: does not look for "nearest" but for first - need to fix.

    // Search ONLY instances within which the mouse is located, as there are no overlapping labels.

    // Search the best from
    int best_from_dist;
    int best_to_dist;

    for(unsigned i=0;i<labels.size();i++)
    {
        // Check if "position" is within range of this label
        if((int)labels[i].from<=position && position<=(int)labels[i].to)
        {
            // Position is within this label. Check proximity to edges.
            //printf("Mouse within instance %d\n",i);

            best_from_dist=-1;
            if( (position-(int)labels[i].from < tolerance) && (position-(int)labels[i].from>=0 ) )
                best_from_dist = position-(int)labels[i].from;
            best_to_dist=-1;
            if( ((int)labels[i].to-position < tolerance) && ((int)labels[i].to-position >=0 ) )
                best_to_dist = (int)labels[i].to-position;

            if(best_from_dist==-1 && best_to_dist==-1)
            {
                // Nothing in range - returns nothing was found
                //printf("No edge in range\n");
                return ig;
            }
            ig.instance=i;
            // Both in range - find the best
            if(best_from_dist!=-1 && best_to_dist!=-1)
            {

                if(best_from_dist<best_to_dist)
                {
                    ig.isfrom=true;
                    ig.at=labels[ig.instance].from;
                }
                else
                {
                    ig.isfrom=false;
                    ig.at=labels[ig.instance].to;
                }
                //printf("Both edges in range. Best is %s dist %d (out of %d and %d)\n",ig.isfrom?"from":"to",abs(ig.at-position),best_from_dist,best_to_dist);
                return ig;
            }
            // Only one of the two in range, find which one
            if(best_from_dist!=-1)
            {
                ig.isfrom=true;
                ig.at=labels[ig.instance].from;
                //printf("Only from edge in range\n");
                return ig;
            }
            ig.isfrom=false;
            ig.at=labels[ig.instance].to;
            //printf("Only to edge in range\n");
            return ig;
        }
    }



    return ig;
}


void MainWindow::on_action_Undo_label_change_triggered()
{
    printf("Current labels:\n");
    printLabelStructure(labels);
    printf("Number of backup levels: %d\n",labels_back.size());
    if(labels_back.size())
    {
        printf("Last backup labels:\n");
        printLabelStructure(labels_back.back());
    }

    if(labels_back.size()==0)
    {
        //QMessageBox::information(this,"Undo","Nothing more to undo");
        terminalPrint("Nothing more to undo\n");
        return;
    }
    labels=labels_back.back();
    labels_back.pop_back();

    terminalPrint("Undoing...\n");

    // Here: must regenerate all the label channel
    updateLabelTraceFromLabelStruct(-1);

    // Repaint the scopes
    viewsRepaint();

    // Update the label table - but flag that the scope location should not change
    populateLabelTable();


}
void MainWindow::instanceChange(INSTANCEGRABBED ig,int newpos)
{
    // Sanity
    assert(ig.instance>=0);
    assert(ig.instance<(int)labels.size());

    // Sanity on range of newpos
    if(newpos<0) newpos=0;
    if(newpos>=(int)dataset.sy) newpos=dataset.sy-1;

    // Backup of change
    labels_back.push_back(labels);

    // Temp
    int oldfrom=labels[ig.instance].from;
    int oldto=labels[ig.instance].to;
    // Change the label
    if(ig.isfrom)
        labels[ig.instance].from = newpos;
    else
        labels[ig.instance].to = newpos;

    // Print something in terminal
    terminalPrint(QString("Updated instance %1 from %2-%3 to %4-%5\n").arg(ig.instance).arg(oldfrom).arg(oldto).arg(labels[ig.instance].from).arg(labels[ig.instance].to));


    // Here: must regenerate all the label channel
    updateLabelTraceFromLabelStruct(ig.instance);

    // Repaint the scopes
    viewsRepaint();


    // Recompute the label structure from the label channel (sanity as struct can have overlapping labels)
    createInstanceStructureFromDataset();



    // Update the label table - but flag that the scope location should not change
    populateLabelTable();


}
void MainWindow::viewsRepaint()
{
    labelview->update();
    for(unsigned i=0;i<scopeviews.size();i++)
        scopeviews[i]->update();
}
void MainWindow::updateLabelTraceFromLabelStruct(int updatelast)
{
    // Re-create the label trace from the instance structure
    assert(nulllabels.size()>=1);       // Null label is first element of nulllabels

    // Null-class all
    for(unsigned y=0;y<dataset.sy;y++)
    {
        //dataset.data[labelchannel][y] = 0;
        dataset.data[labelchannel][y] = nulllabels[0];
    }
    for(unsigned i=0;i<labels.size();i++)
    {
        int y1=labels[i].from;
        int y2=labels[i].to;

        for(int y=y1;y<=y2;y++)
        {
            dataset.data[labelchannel][y] = labels[i].label;
        }
    }
    // updatelast: if specified, this label has the priority over all the other ones: typically the most recently changed
    if(updatelast<0)
        return;
    int y1=labels[updatelast].from;
    int y2=labels[updatelast].to;

    for(int y=y1;y<=y2;y++)
    {
        dataset.data[labelchannel][y] = labels[updatelast].label;
    }
}
void MainWindow::instanceAdd(int from,int to,int label)
{
    // Sanity on ranges
    if(from>to)
    {
        int t=from;
        from=to;
        to=t;
    }
    if(from<0) from=0;
    if(from>=(int)dataset.sy) from=dataset.sy-1;
    if(to<0) to=0;
    if(to>=(int)dataset.sy) from=dataset.sy-1;

    // Backup of change
    labels_back.push_back(labels);

    // Print something in terminal
    terminalPrint(QString("New instance: %1-%2 label %3\n").arg(from).arg(to).arg(label));
    printf("Adding label %d-%d: %d\n",from,to,label);

    // Add label by operating on the label trace
    for(int y=from;y<=to;y++)
    {
        dataset.data[labelchannel][y] = label;
    }

    // Repaint the scopes
    viewsRepaint();

    // Recompute the label structure from the label channel (sanity as struct can have overlapping labels)
    createInstanceStructureFromDataset();

    // Update the label table - but flag that the scope location should not change
    populateLabelTable();


}
void MainWindow::instanceDelete(unsigned i)
{
    // Sanity
    assert(i<labels.size());

    // Backup of change
    labels_back.push_back(labels);

    // Print something in terminal
    terminalPrint(QString("Removing instance %1\n").arg(i));
    printf("Removing instance %d\n",i);

    // Remove label
    labels.erase(labels.begin()+i);

    // Here: must regenerate all the label channel
    updateLabelTraceFromLabelStruct(-1);

    // Repaint the scopes
    viewsRepaint();

    // Recompute the label structure from the label channel (sanity as struct can have overlapping labels)
    createInstanceStructureFromDataset();

    // Update the label table - but flag that the scope location should not change
    populateLabelTable();


}
void MainWindow::scopeviewsClear()
{
    for(unsigned i=0;i<scopeviews.size();i++)
    {
        delete scopeviews[i];
        scopeviews[i]=0;
    }
    scopeviews.clear();
}
void MainWindow::viewsClear()
{
    scopeviewsClear();
    if(labelview)
    {
        delete labelview;
        labelview=0;
    }
}
void MainWindow::scopeviewsAddN(unsigned n)
{
    for(unsigned i=0;i<n;i++)
    {
        SSViewScope* scopeview = new SSViewScope(this,scale);
        scopeviews.push_back(scopeview);
        ui->uif_plots->layout()->addWidget(scopeview);
        connect(scopeview,SIGNAL(timeChanged(int)),this,SLOT(on_View_timeChanged(int)));
        connect(scopeview,SIGNAL(mousePressed(Qt::MouseButton,int)),this,SLOT(on_View_mousePressed(Qt::MouseButton,int)));
        connect(scopeview,SIGNAL(mouseReleased(Qt::MouseButton,int)),this,SLOT(on_View_mouseReleased(Qt::MouseButton,int)));
        connect(scopeview,SIGNAL(mouseMoved(int)),this,SLOT(on_View_mouseMoved(int)));
        connect(scopeview,SIGNAL(keyPressed(int,int)),this,SLOT(on_View_keyPressed(int,int)));
        connect(scopeview,SIGNAL(Leaved()),this,SLOT(on_View_Leaved()));
        connect(scopeview,SIGNAL(zoomHInned()), this,SLOT(on_View_zoomHInned()));
        connect(scopeview,SIGNAL(zoomHOuted()), this,SLOT(on_View_zoomHOuted()));
        connect(scopeview,SIGNAL(zoomHReseted()), this,SLOT(on_View_zoomHReseted()));

    }
}
/**
  \brief Load a data source from a file. Return format: data[channel][samples]

  rv: return value.
      0: success
      -1: file not found
      -2: invalid file content

   Parses signed integers space or tab separated. Handles special entries of the form 'NaN' and replaces them by zeroes.
**/
std::vector<std::vector<int> > MainWindow::LoadSignalFile(std::string file,int &rv, bool report)
{
    (void) report;
   std::vector<std::vector<int> > data;

   rv=0;

   std::vector<int> d;

   QFile filein(QString(file.c_str()));
   if (filein.open(QIODevice::ReadOnly))
   {
      QByteArray filedata = filein.readAll();
      bool ok;
      data = parse(filedata.data(),ok);
      if(!ok)
      {
         rv=-2;
         return data;
      }

   }
   else
   {
      rv=-1;
      return data;
   }

   return data;
}
std::vector<std::vector<int> > MainWindow::LoadSignalFromByteArray(QByteArray &filedata,int &rv, bool report)
{
    (void) report;
    std::vector<std::vector<int> > data;
    rv=0;
    bool ok;
    data = parse(filedata.data(),ok);
    if(!ok)
    {
       rv=-2;
       return data;
    }

    return data;
}

void MainWindow::on_action_How_to_triggered()
{
    QFile file(":/res/howto.html");
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QByteArray filedata = file.readAll();
    HelpWindow dialog(QString(filedata),this);
 #ifdef Q_OS_ANDROID
     dialog.setWindowState(dialog.windowState() | Qt::WindowMaximized);
 #endif
    dialog.exec();
}

/*
 * Convert a space, colon or semicolon list of ints into a vector.
 * Return:  0=ok
 *          nonzero=error
*/
int MainWindow::nulllabelStringToVec(QString str,std::vector<int> &nl)
{
    // Split on separators
    QStringList list = str.split(QRegExp("[\\s,;]+"),Qt::SkipEmptyParts);

    // Print the content
#if 0
    for(int i=0;i<list.size();i++)
    {
        printf("%d: '%s'\n",i,list.at(i).toStdString().c_str());
    }
    printf("\n");
#endif
    // Check that all is numerical and convert to numerical
    std::vector<int> list_int;
    bool convok;
    bool allok=true;
    for(int i=0;i<list.size();i++)
    {
        list_int.push_back(list.at(i).toInt(&convok));
        allok=allok&convok;     // AND operation
    }
    if(allok==false)
    {
        return 1;
    }
    nl = list_int;
    return 0;
}
QString MainWindow::nulllabelVecToString(std::vector<int> nulllabels)
{
    QString str="";
    for(unsigned i=0;i<nulllabels.size();i++)
    {
        str+=QString("%1 ").arg(nulllabels[i]);
    }
    return str;
}
/******************************************************************************
  saveSettings
*******************************************************************************
Save the app configuration to QSettings.
Returns 0 in case of success
******************************************************************************/
int MainWindow::saveSettings(QString ini)
{

    QSettings *settings;

    if(ini==QString())
        settings=new QSettings(QSettings::IniFormat,QSettings::UserScope,"danielroggen","labeller");
    else
        settings=new QSettings(ini,QSettings::IniFormat);

    if(settings->status()!=QSettings::NoError)
        return true;


    printf("Storing settings to %s\n",settings->fileName().toStdString().c_str());
    terminalPrint(QString("Storing settings to %1\n").arg(settings->fileName()));

    // Only 4 things to save: format string; nullclass; current label; filename

    settings->setValue("FormatString",formatstring);
    settings->setValue("NullLabels",nulllabelVecToString(nulllabels));
    settings->setValue("LabelChannel",labelchannel);
    settings->setValue("Datafile",QString::fromStdString(dataset.filename));        // Absolute data

    delete settings;

    return false;
}



void MainWindow::on_action_Save_project_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save settings",QString(),"Labelling project (*.lpb)");
    if(!fileName.isNull())
    {
        bool rv = saveSettings(fileName);
        if(rv)
            QMessageBox::critical(this, "Save settings error", "Cannot write to file");
    }
}

void MainWindow::on_action_Open_project_triggered()
{
    // Open project
    QString fileName = QFileDialog::getOpenFileName(this, "Load settings",QString(),"Labelling project (*.lpb)");
    if(!fileName.isNull())
       loadSettings(fileName);
}
int MainWindow::loadSettings(QString fileName)
{


    QSettings *settings;

    if(fileName==QString())
        settings = new QSettings(QSettings::IniFormat,QSettings::UserScope,"danielroggen","dscopeqt");
    else
        settings = new QSettings(fileName,QSettings::IniFormat);

    if(settings->status()!=QSettings::NoError)
        return true -1;

    // Clear all dataset and state variables
    clearAllState();
    // Set the UI to no dataset (deactivate the UI elements)
    uiSetToNoDataset();


    QString s_formatstring = settings->value("FormatString","").toString();
    QString s_nulllabelstr = settings->value("NullLabels","0").toString();
    int s_labelchannel = settings->value("LabelChannel",0).toInt();
    QString datafile = settings->value("Datafile","").toString();

    // Try load dataset; failure: return
    if(loadDataset(datafile)!=0)
        return 1;

    // Fill the
    ui->uile_null_label->setText(s_nulllabelstr);
    ui->uile_formatstring->setText(s_formatstring); formatstring=s_formatstring;
    labelchannel=s_labelchannel;
    // Limit the label spin box to the range of channels
    ui->uisb_labelchannel->setMaximum(dataset.sx-1);
    ui->uisb_labelchannel->setValue(labelchannel);

    on_uipb_apply_clicked();

    uiSetToDataset();


    return 0;
}

void MainWindow::on_action_About_triggered()
{
    QMessageBox::about(this, "About",
    "<p><b>Labeller</b></p>\n"
    "<p>Version 0.9</p>"
    "<p>(c) 2020 Daniel Roggen</p>");
}

void MainWindow::on_action_Export_dataset_triggered()
{
    //exportSync();
    exportAsync();
}
void MainWindow::exportSync()
{
    // Export annotated dataset
    QString t="Save data";
    QString fileName = QFileDialog::getSaveFileName(this,t,QString::fromStdString(dataset.filename),"Text (*.txt)");
    if(fileName.isNull())
        return;

    printf("Saving to %s\n",fileName.toStdString().c_str());


   QFile file(fileName);
   if(!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
   {
       QMessageBox::critical(this,t+" error", "Cannot write to file");
       return;
   }

    // Stream writer...
    QTextStream out(&file);
    for(unsigned y=0;y<dataset.sy;y++)
    {
        QString s = "";
        for(unsigned x=0;x<dataset.sx;x++)
        {
            s+=QString("%1 ").arg(dataset.data[x][y]);
        }
        s+="\n";
        out << s;
    }
    out.flush();
    file.close();
}
void MainWindow::exportAsync()
{
    // Create the data buffer
    printf("%d %d\n",dataset.sx,dataset.sy);
    QByteArray ba;
    // Stream writer...
    QTextStream out(&ba);
    for(unsigned y=0;y<dataset.sy;y++)
    {
        QString s = "";
        for(unsigned x=0;x<dataset.sx;x++)
        {
            s+=QString("%1 ").arg(dataset.data[x][y]);
        }
        s+="\n";
        out << s;
    }
    out.flush();

    printf("ba size: %d\n",ba.size());

    // Prompt to save the data
    QFileDialog::saveFileContent(ba, QString::fromStdString(dataset.filename));


}
void MainWindow::on_View_zoomHInned()
{
    QObject *s = sender();
    //printf("MainWindow::on_View_zoomHInned\n");
    if(labelview!=s)
        labelview->zoomHIn();

    for(unsigned i=0;i<scopeviews.size();i++)
    {
        if(scopeviews[i]!=s)
            scopeviews[i]->zoomHIn();
    }
}
void MainWindow::on_View_zoomHOuted()
{
    QObject *s = sender();
    //printf("MainWindow::on_View_zoomHOuted\n");
    if(labelview!=s)
        labelview->zoomHOut();

    for(unsigned i=0;i<scopeviews.size();i++)
    {
        if(scopeviews[i]!=s)
            scopeviews[i]->zoomHOut();
    }
}
void MainWindow::on_View_zoomHReseted()
{
    QObject *s = sender();
    //printf("MainWindow::on_View_zoomHReseted\n");
    if(labelview!=s)
        labelview->zoomHReset();

    for(unsigned i=0;i<scopeviews.size();i++)
    {
        if(scopeviews[i]!=s)
            scopeviews[i]->zoomHReset();
    }
}

void MainWindow::on_action_Add_column_triggered()
{
    //printf("Add column\n");
    //addColumnSync();
    addColumnAsync();
}
void MainWindow::addColumnSync()
{

    // BUG: webassembly
    // http://qtandeverything.blogspot.com/2019/05/exec-on-qt-webassembly.html

    // Sanity
    assert(dataset.sx>=1);
    assert(dataset.sy>=1);

    bool ok;
    int col = dialogGetLabelID(ok,QString("New column location (0 is leftmost; %1 is rightmost)").arg(dataset.sx),"Enter new column location");
    if(!ok)
        return;
    // Check that the column is within range
    if(col<0 || col>(int)dataset.sx)
    {
        QMessageBox::critical(this,"Error",QString("Invalid column number; it must be in the range [0;%1]").arg(dataset.sx));
        return;
    }
    int label = dialogGetLabelID(ok);
    if(!ok)
        return;

    addColumnAt(col,label);
}
// Add the column
void MainWindow::addColumnAt(int col,int label)
{

    printf("Adding label column at %d with label value %d\n",col,label);

    std::vector<int> v(dataset.sy,label);   // New data column
    dataset.data.insert(dataset.data.begin()+col,v);
    dataset.sx = dataset.data.size();

    terminalPrint(QString("Added column at %1 with label value %2\n").arg(col).arg(label));

    // Must update UI to reflect the additional column
    uiSetToDataset();

    // Clear label only
    clearLabelStructures();

    // Limit the label spin box to the range of channels
    ui->uisb_labelchannel->setMaximum(dataset.sx-1);

    viewsCreate();
}

void MainWindow::addColumnAsync()
{
    // Sanity
    assert(dataset.sx>=1);
    assert(dataset.sy>=1);


    // Asynchronous call to dialog boxes for webassembly
    std::function<void (bool,int)> callback=[=](bool ok, int col) {
                    printf("addColumnAsync lambda callback: ok: %d v: %d\n",(int)ok,col);
                    if(ok==false)
                        return;
                    // Column was returned, so create second dialog
                    if(col<0 || col>(int)dataset.sx)
                    {
                        QMessageBox::critical(this,"Error",QString("Invalid column number; it must be in the range [0;%1]").arg(dataset.sx));
                        return;
                    }
                    // Column is ok - get the label

                    std::function<void (bool,int)> callback2=[=](bool ok, int label) {
                        printf("addColumnAsync lambda callback 2: ok: %d v: %d\n",(int)ok,label);

                        if(ok)
                        {
                            addColumnAt(col,label);
                        }
                    };

                    dialogGetLabelID2(callback2);


                };


    dialogGetLabelID2(callback,QString("New column location (0 is leftmost; %1 is rightmost)").arg(dataset.sx),"Enter new column location");

}


void MainWindow::on_pushButton_clicked()
{
    // Try some dialog
/*
    std::function<void (bool,int)> callback=[=](bool ok, int v) {
                    printf("lambda callback: ok: %d v: %d\n",(int)ok,v);
                };


    dialogGetLabelID2(callback,"Test input","Test title");
*/
    addColumnAsync();
}
