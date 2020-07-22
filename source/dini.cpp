#include <Qt>
#include <QString>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QFileInfo>
#include "dini.h"


int DIni::load(const QString fname,QMap<QString, QString> &data)
{
    QFile file(fname);
    if(!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::information(0, "Error", file.errorString());
        return -1;
    }

    printf("file to open: %s\n",fname.toStdString().c_str());

    QByteArray ba = file.readAll();


    DIni::loadFromByteArray(ba,data);

    return 0;
}
void DIni::loadFromByteArray(const QByteArray &ba,QMap<QString, QString> &data)
{
    data.clear();
    QTextStream in(ba);

    while(!in.atEnd())
    {
        QString line = in.readLine();

        // Trim line
        line = line.trimmed();

        // Check if line is a comment or empty
        if(line.startsWith("#") || line.startsWith("rem",Qt::CaseInsensitive) || line.isEmpty())
        {
            continue;
        }
        // Check if line is a [sentence in brackets] - INI files use this, but not implemented here
        if(line.startsWith("["))
        {
            printf("Skipping group: '%s'\n",line.toStdString().c_str());
            continue;
        }

        printf("Line: '%s'\n",line.toStdString().c_str());


        // If the line has no '=' then skip

        int sp = line.indexOf("=");
        if(sp==-1)
        {
            printf("Skipping as no assignment: '%ss'\n",line.toStdString().c_str());
            continue;
        }
        // If the line has a '=' then split in two on the leftmost '='
        QString l = line.left(sp);
        QString r = line.right(line.size()-1-sp);

        // If any is empty then skip
        if(l.isEmpty() || r.isEmpty())
        {
            printf("Skipping as no token left or right of =: '%s'\n",line.toStdString().c_str());
            continue;
        }

        // If the value is enclosed in quotation marks (e.g. when saved with QSettings) then remove the quotes
        if(r.startsWith("\"") && r.endsWith("\""))
        {
            r = r.mid(1,r.size()-2);
        }

        // Store the pair of token l,r in a structure
        data[l] = r;
    }

}
int DIni::save(const QString filename, const QMap<QString, QString> &data)
{
    QFile file(filename);
    if(!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::information(0, "Error", file.errorString());
        return -1;
    }

    printf("file to open: %s\n",filename.toStdString().c_str());

    QByteArray ba;
    DIni::saveToByteArray(data,ba);

    file.write(ba);

    file.close();


    return 0;
}
void DIni::saveToByteArray(const QMap<QString, QString> &data,QByteArray &ba)
{
    ba.clear();
    QTextStream out(&ba);

    // Print the content of the map
    QMapIterator<QString, QString> it(data);
    while (it.hasNext())
    {
        it.next();
        printf("'%s' = '%s'\n",it.key().toStdString().c_str(),it.value().toStdString().c_str());

        QString str=QString("%1=\"%2\"\n").arg(it.key()).arg(it.value());
        out << str;
    }
    out.flush();
}
void DIni::print(QMap<QString,QString> &data)
{
    // Print the content of the map
    QMapIterator<QString, QString> it(data);
    while (it.hasNext())
    {
        it.next();
        printf("'%s' = '%s'\n",it.key().toStdString().c_str(),it.value().toStdString().c_str());
    }
}
