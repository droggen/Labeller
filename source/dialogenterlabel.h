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

protected:
    void done(int r) override;

private:
    Ui::DialogEnterLabel *ui;

signals:
    void dataEntered(bool ok,int value);
};

#endif // DIALOGENTERLABEL_H
