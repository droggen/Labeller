#ifndef DIALOGENTERLABEL_H
#define DIALOGENTERLABEL_H

#include <QDialog>

namespace Ui {
class DialogEnterLabel;
}

class DialogEnterLabel : public QDialog
{
    Q_OBJECT

public:
    explicit DialogEnterLabel(QWidget *parent = nullptr);
    ~DialogEnterLabel();

    void setExplanation(QString str,QString tittle);
    int getLabel(bool &ok);

private:
    Ui::DialogEnterLabel *ui;
};

#endif // DIALOGENTERLABEL_H
