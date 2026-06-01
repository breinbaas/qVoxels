#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <proj.h>

#include "helpers.h"

WGS84Coord convertRDToWGS84(double x, double y) {
    PJ_CONTEXT *C = proj_context_create();

    QString path = QCoreApplication::applicationDirPath() + "/proj_data";
    std::string nativePath = QDir::toNativeSeparators(path).toStdString();

    const char* paths[] = { nativePath.c_str() };
    proj_context_set_search_paths(C, 1, paths);

    PJ *P = proj_create_crs_to_crs(C, "EPSG:28992", "EPSG:4326", nullptr);

    if (P == nullptr) {
        qCritical() << "Failed to create PROJ transformation object.";
        proj_context_destroy(C);
        return {0.0, 0.0};
    }

    PJ *P_for_gis = proj_normalize_for_visualization(C, P);
    proj_destroy(P); // Free the original, use the normalized one

    PJ_COORD input = proj_coord(x, y, 0, 0);

    PJ_COORD output = proj_trans(P_for_gis, PJ_FWD, input);

    proj_destroy(P_for_gis);
    proj_context_destroy(C);

    // output.xy.x is Longitude, output.xy.y is Latitude
    return { output.xy.y, output.xy.x };
}
