#ifndef DINI_H
#define DINI_H

#include <QString>
#include <QMap>

class DIni
{
public:
    //DIni();


    static int load(const QString filename, QMap<QString, QString> &data);
    static void print(QMap<QString,QString> &data);
    static int save(const QString filename, const QMap<QString, QString> &data);

    static void saveToByteArray(const QMap<QString, QString> &data,QByteArray &ba);
    static void loadFromByteArray(const QByteArray &ba,QMap<QString, QString> &data);
};

#endif // DINI_H
