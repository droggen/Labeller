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
