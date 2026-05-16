#include "cpt.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <cmath>

Cpt::Cpt(QObject *parent) : QObject(parent)
{
}

Cpt* Cpt::fromGef(const QString &gefFilePath, QObject *parent)
{
    QFile file(gefFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file for reading:" << gefFilePath;
        return nullptr;
    }

    Cpt *cpt = new Cpt(parent);
    GefMetadata metadata;
    double top = 0.0;
    bool readingHeader = true;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();

        if (readingHeader) {
            // Replicating Python logic: if line doesn't contain "#ZID" (Note: check logic flow)
            // Python: if line.find("#ZID") < 0: try to extract top.
            if (line.contains("#ZID")) {
                QStringList args = line.split("=");
                if (args.size() > 1) {
                    QStringList subArgs = args[1].split(",");
                    if (subArgs.size() > 1) {
                        bool ok;
                        top = subArgs[1].trimmed().toDouble(&ok);
                        if (!ok) {
                            qWarning() << "Error reading zid top value from line:" << line;
                            delete cpt;
                            return nullptr;
                        }
                    }
                }
            }

            if (line.contains("#EOH")) {
                readingHeader = false;
            } else {
                try {
                    parseHeaderLine(cpt, line, metadata);
                } catch (const std::exception &e) {
                    qWarning() << e.what();
                    delete cpt;
                    return nullptr;
                }
            }
        } else {
            try {
                parseDataLine(cpt, line, metadata, top);
            } catch (const std::exception &e) {
                qWarning() << e.what();
                delete cpt;
                return nullptr;
            }
        }
    }

    file.close();

    // Post processing: Calculate FR
    cpt->m_fr.clear();
    for (int i = 0; i < cpt->m_qc.size(); ++i) {
        double qc = cpt->m_qc[i];
        double fs = cpt->m_fs[i];
        if (qc == 0.0) {
            cpt->m_fr.append(CPT_FR_MAX);
        } else {
            cpt->m_fr.append((fs / qc) * 100.0);
        }
    }

    // Post processing: Remove NaN lines
    QList<double> zs, qcs, fss, frs, u2s;
    for (int i = 0; i < cpt->m_z.size(); ++i) {
        double z = cpt->m_z[i];
        double qc = cpt->m_qc[i];
        double fs = cpt->m_fs[i];
        double fr = cpt->m_fr[i];
        double u2 = cpt->m_u2[i];

        if (std::isnan(z)) {
            continue;
        }
        if (std::isnan(qc)) qc = 0.0;
        if (std::isnan(fs)) fs = 0.0;
        if (std::isnan(fr)) fr = 0.0;
        if (std::isnan(u2)) u2 = 0.0;

        zs.append(z);
        qcs.append(qc);
        fss.append(fs);
        frs.append(fr);
        u2s.append(u2);
    }

    cpt->m_z = zs;
    cpt->m_qc = qcs;
    cpt->m_fs = fss;
    cpt->m_fr = frs;
    cpt->m_u2 = u2s;

    return cpt;
}

void Cpt::parseHeaderLine(Cpt *cpt, const QString &line, GefMetadata &metadata)
{
    QStringList args = line.split("=");
    if (args.size() < 2) {
        return; // Skip or handle malformed lines safely
    }

    QString keyword = args[0].trimmed().replace("#", "");
    QString argline = args[1].trimmed();
    QStringList argList = argline.split(",");

    if (keyword == "PROCEDURECODE" || keyword == "REPORTCODE") {
        if (argList[0].toUpper().contains("BORE")) {
            throw std::runtime_error("This is a borehole file instead of a Cpt file");
        }
    }
    else if (keyword == "RECORDSEPARATOR") {
        metadata.recordSeparator = argList[0];
    }
    else if (keyword == "COLUMNSEPARATOR") {
        metadata.columnSeparator = argList[0];
    }
    else if (keyword == "COLUMNINFO") {
        if (argList.size() >= 4) {
            int column = argList[0].toInt();
            int dtype = argList[3].trimmed().toInt();
            if (dtype == GEF_COLUMN_Z_CORRECTED) {
                dtype = GEF_COLUMN_Z;
            }
            metadata.columnInfo[dtype] = column - 1;
        }
    }
    else if (keyword == "XYID") {
        if (argList.size() >= 3) {
            cpt->m_x = std::round(argList[1].trimmed().toDouble() * 100.0) / 100.0;
            cpt->m_y = std::round(argList[2].trimmed().toDouble() * 100.0) / 100.0;
        }
    }
    else if (keyword == "MEASUREMENTVAR") {
        if (argList.size() >= 2 && argList[0] == "13") {
            cpt->m_preExcavatedDepth = argList[1].toDouble();
        }
    }
    else if (keyword == "COLUMNVOID") {
        if (argList.size() >= 2) {
            int col = argList[0].trimmed().toInt();
            metadata.columnVoids[col - 1] = argList[1].trimmed().toDouble();
        }
    }
    else if (keyword == "TESTID") {
        cpt->m_name = argList[0].trimmed();
    }
    else if (keyword == "FILEDATE" || keyword == "STARTDATE") {
        if (argList.size() >= 3) {
            int yyyy = argList[0].trimmed().toInt();
            int mm = argList[1].trimmed().toInt();
            int dd = argList[2].trimmed().toInt();

            if (yyyy >= 1900 && yyyy <= 2100 && mm >= 1 && mm <= 12 && dd >= 1 && dd <= 31) {
                cpt->m_date = QString("%1%2%3")
                .arg(yyyy)
                    .arg(mm, 2, 10, QChar('0'))
                    .arg(dd, 2, 10, QChar('0'));
            } else {
                cpt->m_date = "";
            }
        }
    }
}

void Cpt::parseDataLine(Cpt *cpt, const QString &line, const GefMetadata &metadata, double top)
{
    if (line.trimmed().isEmpty()) {
        return;
    }

    // Clean record separator out
    QString cleanLine = line;
    if (!metadata.recordSeparator.isEmpty()) {
        cleanLine.replace(metadata.recordSeparator, "");
    }
    cleanLine = cleanLine.trimmed();

    // Split based on column separator
    QStringList rawArgs = cleanLine.split(metadata.columnSeparator, Qt::SkipEmptyParts);
    QList<double> args;
    for (const QString &arg : rawArgs) {
        QString trimmed = arg.trimmed();
        if (!trimmed.isEmpty() && trimmed != metadata.recordSeparator) {
            args.append(trimmed.toDouble());
        }
    }

    if (args.isEmpty()) return;

    // Skip lines that hit a columnvoid value
    for (auto it = metadata.columnVoids.constBegin(); it != metadata.columnVoids.constEnd(); ++it) {
        int colIndex = it.key();
        double voidValue = it.value();
        if (colIndex < args.size() && args[colIndex] == voidValue) {
            return;
        }
    }

    int zcolumn = metadata.columnInfo.value(GEF_COLUMN_Z, -1);
    int qccolumn = metadata.columnInfo.value(GEF_COLUMN_QC, -1);
    int fscolumn = metadata.columnInfo.value(GEF_COLUMN_FS, -1);
    int ucolumn = metadata.columnInfo.value(GEF_COLUMN_U, -1);

    if (zcolumn != -1 && zcolumn < args.size()) {
        double dz = top - std::abs(args[zcolumn]);
        cpt->m_z.append(dz);
    }

    if (qccolumn != -1 && qccolumn < args.size()) {
        double qc = args[qccolumn];
        if (qc <= 0) qc = 1e-3;
        cpt->m_qc.append(qc);
    }

    if (fscolumn != -1 && fscolumn < args.size()) {
        double fs = args[fscolumn];
        if (fs <= 0) fs = 1e-6;
        cpt->m_fs.append(fs);
    }

    if (ucolumn != -1 && ucolumn < args.size()) {
        cpt->m_u2.append(args[ucolumn]);
    } else {
        cpt->m_u2.append(0.0);
    }
}