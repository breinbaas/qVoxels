#ifndef SOILCOLORPALETTE_H
#define SOILCOLORPALETTE_H

#include <QObject>
#include <QMap>
#include <QColor>

class SoilColorPalette {

public:
    explicit SoilColorPalette(QObject *parent = nullptr){
        // Hardcoded defaults based on your dict
        m_palette["preexcavated"] = QColor("#6f6664");
        m_palette["organic_clay"] = QColor("#32e052");
        m_palette["clay"]         = QColor("#034b10");
        m_palette["silty_clay"]   = QColor("#608233");
        m_palette["silty_sand"]   = QColor("#d6e119");
        m_palette["sand"]         = QColor("#fef341");
        m_palette["dense_sand"]   = QColor("#fff000");
        m_palette["peat"]         = QColor("#7b530b");
    }


    QColor getColor(const QString &soilCode) const {
        return m_palette.value(soilCode, QColor("#FFFFFF"));
    }

private:
    QMap<QString, QColor> m_palette;
};

#endif // SOILCOLORPALETTE_H