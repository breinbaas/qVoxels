#ifndef BOREHOLE_H
#define BOREHOLE_H

#include <QObject>

class Borehole : public QObject
{
    Q_OBJECT
public:
    explicit Borehole(QObject *parent = nullptr);

signals:
};

#endif // BOREHOLE_H
