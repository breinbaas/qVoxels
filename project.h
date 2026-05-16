#ifndef PROJECT_H
#define PROJECT_H

#include <QObject>
#include <QString> // Include QString for m_name and m_path

#include "cpt.h"
#include "borehole.h"

class Project : public QObject
{
    Q_OBJECT

private:
    QString m_name; // Project name
    QString m_path; // Project base path
    QList<Cpt> m_cpts;
    QList<Borehole> m_boreholes;

    bool m_isDirty;
public:
    explicit Project(QObject *parent = nullptr);

    static Project* fromPath(const QString &projectPath, QObject *parent = nullptr);

    // Getters
    bool isDirty() const;
    QString name() const;
    QString path() const;

    // Setters
    void setDirty(bool dirty);
    void setName(const QString& name);
    void setPath(const QString& path);

signals:
    void dirtyChanged(bool dirty);
};

#endif // PROJECT_H