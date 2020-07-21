#include "dialogenterlabel.h"
#include "ui_dialogenterlabel.h"

DialogEnterLabel::DialogEnterLabel(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogEnterLabel)
{
    ui->setupUi(this);
}

DialogEnterLabel::~DialogEnterLabel()
{
    delete ui;
}

void DialogEnterLabel::setExplanation(QString str,QString title)
{
    ui->uil_explanation->setText(str);
    setWindowTitle(title);
}
int DialogEnterLabel::getLabel(bool &ok)
{
    bool _ok;
    int v = ui->uile_label->text().toInt(&_ok);
    ok = _ok;
    return v;
}

void DialogEnterLabel::done(int r)
{
    // r=0: cancel
    // r=1: ok

    printf("DialogEnterLabel::done %d\n",r);

    bool ok;
    int v = getLabel(ok);

    if(r==1)
    {
        printf("DialogEnterLabel: emit %d %d\n",(int)ok,v);
        emit dataEntered(ok,v);
    }
    else
    {
        printf("DialogEnterLabel: emit %d %d\n",(int)false,0);
        emit dataEntered(false,0);
    }

    QDialog::done(r);
}
