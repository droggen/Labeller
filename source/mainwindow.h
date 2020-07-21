#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "data.h"
#include "dataset.h"
#include "helper.h"
#include "dterminal.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_action_Load_triggered();





    // Signal-related stuff
    void on_View_timeChanged(int value);
    void on_View_mousePressed(Qt::MouseButton button, int samplex);
    void on_View_mouseReleased(Qt::MouseButton button, int samplex);
    void on_View_mouseMoved(int samplex);
    void on_View_keyPressed(int key,int samplex);
    void on_View_Leaved();
    void on_View_zoomHInned();
    void on_View_zoomHOuted();
    void on_View_zoomHReseted();





    void on_uisb_labelchannel_valueChanged(int arg1);

    void on_uipb_apply_clicked();

    void on_uitw_labels_itemSelectionChanged();

    void on_uipb_updatelabeltable_clicked();


    void on_action_Undo_label_change_triggered();


    void on_action_How_to_triggered();

    void on_action_About_triggered();

    void on_action_Save_project_triggered();

    void on_action_Open_project_triggered();

    void on_action_Export_dataset_triggered();

    void on_action_Add_column_triggered();

    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;

    DATASET dataset;
    std::vector<LABEL> labels;                              // Contains start-end-label for each instance;
    std::vector<std::vector<LABEL> > labels_back;          // Backup labels
    std::vector<int> nulllabels;
    unsigned labelchannel;

    std::vector<SSViewScope*> scopeviews;
    SSViewLabel *labelview;

    SSViewLabelConfig viewlabelconfig;      // Visual parameters and pointer to data

    ScopesDefinition scopedefinitions;      // Includes what to visualise in the scope: channels, scale, etc



    DTerminal terminal;

    QCursor curleft,curright;

    int proximitytolerance;         // Tolerance for snapping to edge of labels

    bool isgrabbing;                    // Indicate if an instance is being grabbed
    bool ispainting;                    // Indicate if a new instance is being "painted"
    bool islabelconfirmed;              // Only allow label edits when labels are confirmed
    // TODO: iswithingrabbingrange may not be needed
    INSTANCEGRABBED instanceedited;    // Indicates which instance is grabbed

    int ignoreTableSelectN;         // If nonzero, up to N changes in the label table select won't change the scope location

    QString formatstring;           // Only used to store the last valid format string to save settings

    unsigned scale;

    // ---------------------------------------------------------
    // UI   UI   UI   UI   UI   UI   UI   UI   UI   UI   UI   UI
    void uiSetToNoDataset();
    int uiSetToDataset();

    // ---------------------------------------------------------
    // DATASET   DATASET   DATASET   DATASET   DATASET   DATASET
    bool loadDataset(QString filename);
    void clearAllState();
    void uiPrepareDefaultFromDataset();
    void AddScopeView(SCOPEVIEWDATA &vd);
    void DeleteScopeViews();
    void AddScopeViewData(const SCOPEVIEWDATA &view);
    unsigned GetNumScopeViews();
    void PrepareDataPlotScope(VIEWID view,vector<vector<int> *> &vd,vector<unsigned> &vc);

    void addColumnSync();
    void addColumnAsync();

    void printScopedefs(ScopesDefinition sd);
    QString createDefaultFormatStringFromDataset(QString filename);

    void scopesCreateFromScopeDefinitions();           // Create a scope widget to see the data
    void addViewLabel();             // Create a label widget to see the labels
    void populateLabelTable();
    void createInstanceStructureFromDataset();
    void printLabelStructure(std::vector<LABEL> label);
    void updateLabels();
    void viewsCreate();
    void terminalPrint(QString s);
    void terminalClear();
    void printLabelStat();
    INSTANCEGRABBED checkProximityInstance(int position, int tolerance);
    void changeCursorGrabbing(INSTANCEGRABBED ig);
    // Add, edit, delete instances
    void instanceChange(INSTANCEGRABBED ig,int newpos);
    void instanceDelete(unsigned i);
    void instanceAdd(int from,int to,int label);

    void updateLabelTraceFromLabelStruct(int updatelast);

    int dialogGetLabelID(bool &ok, QString expl="Label ID:", QString title="Enter the label ID");
    //int dialogGetLabelID2(bool &ok, QString expl="Label ID:", QString title="Enter the label ID", std::function<bool, int> callback);
    void dialogGetLabelID2(std::function<void (bool,int)> callback, QString expl="Label ID:", QString title="Enter the label ID");
    void setLabelChannel(int arg1);

    int checkFormatString(ScopesDefinition &scopesdef);
    int checkNullLabels(std::vector<int> &nl);
    int scopeviewsParseFormat(QString format,ScopesDefinition &scopedefs);
    void scopeviewsClear();
    void scopeviewsAddN(unsigned n);
    void viewsClear();
    void viewsRepaint();

    void setTime(int value);
    void clearLabelStructures();

    std::vector<std::vector<int> > LoadSignalFile(std::string file,int &rv, bool report);
    std::vector<std::vector<int> > LoadSignalFromByteArray(QByteArray &filedata, int &rv, bool report);

    int nulllabelStringToVec(QString str,std::vector<int> &nl);
    QString nulllabelVecToString(std::vector<int> nulllabels);

    int saveSettings(QString ini);
    int loadSettings(QString fileName);

    void loadPromptSync();
    void loadPromptAsync();
    static void toto(const QString &fn, const QByteArray &ba);
    void loadAsynCompleted(const QString &fileName, const QByteArray &fileContent);
    void exportSync();
    void exportAsync();
};



#endif // MAINWINDOW_H
