#ifndef CPT_H
#define CPT_H

#include <QObject>
#include <QList>
#include <QString>
#include <QMap>

#include "soilprofile.h"

// Constants
const int GEF_COLUMN_Z = 1;
const int GEF_COLUMN_QC = 2;
const int GEF_COLUMN_FS = 3;
const int GEF_COLUMN_U = 6;
const int GEF_COLUMN_Z_CORRECTED = 11;
const double CPT_FR_MAX = 99.9;

class Cpt : public QObject
{
    Q_OBJECT

public:
    explicit Cpt(QObject *parent = nullptr);
    ~Cpt();

    void setSoilProfile(SoilProfile *soilProfile) {m_soilProfile = soilProfile;}
    SoilProfile* soilProfile() const { return m_soilProfile; }

    // Static factory method (Returns nullptr if parsing fails, throwing an exception/error could also be handled via bool)
    static Cpt* fromGef(const QString &gefFilePath, QObject *parent = nullptr);

    // Getters / Setters to access data
    QString name() const { return m_name; }
    QString filePath() const { return m_filePath;}
    double x() const { return m_x; }
    double y() const { return m_y; }
    double latitude() const { return m_latitude; }
    double longitude() const { return m_longitude; }

    QString date() const { return m_date; }
    double preExcavatedDepth() const { return m_preExcavatedDepth; }

    QList<double> z() const { return m_z; }
    QList<double> qc() const { return m_qc; }
    QList<double> fs() const { return m_fs; }
    QList<double> fr() const { return m_fr; }
    QList<double> u2() const { return m_u2; }

    void setFilePath(const QString filePath) {m_filePath = filePath;}

private:
    SoilProfile *m_soilProfile = nullptr;
    QString m_name = "";
    QString m_filePath = "";
    double m_x = 0.0;
    double m_y = 0.0;
    double m_latitude = 0.0;
    double m_longitude = 0.0;
    QString m_date = "";
    double m_preExcavatedDepth = 0.0;

    QList<double> m_z;
    QList<double> m_qc;
    QList<double> m_fs;
    QList<double> m_fr;
    QList<double> m_u2;

    // Internal helper structures for parsing metadata
    struct GefMetadata {
        QString recordSeparator = "";
        QString columnSeparator = " ";
        QMap<int, double> columnVoids;
        QMap<int, int> columnInfo;
    };

    // Helper functions mapped from Python inner functions
    static void parseHeaderLine(Cpt *cpt, const QString &line, GefMetadata &metadata);
    static void parseDataLine(Cpt *cpt, const QString &line, const GefMetadata &metadata, double top);
};

// to make QVariant aware of this class...
Q_DECLARE_METATYPE(Cpt*)

#endif // CPT_H