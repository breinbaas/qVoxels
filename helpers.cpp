#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <proj.h>

#include "helpers.h"

WGS84Coord convertRDToWGS84(double x, double y) {
    // 1. Create a context
    PJ_CONTEXT *C = proj_context_create();

    // Set the path to the data
    QString path = QCoreApplication::applicationDirPath() + "/proj_data";
    std::string nativePath = QDir::toNativeSeparators(path).toStdString();

    const char* paths[] = { nativePath.c_str() };
    proj_context_set_search_paths(C, 1, paths);

    // 2. Create the transformation object using EPSG codes
    // Note: EPSG:4326 expects coordinates in Latitude, Longitude order by default in PROJ 6+
    PJ *P = proj_create_crs_to_crs(C, "EPSG:28992", "EPSG:4326", nullptr);

    if (P == nullptr) {
        qCritical() << "Failed to create PROJ transformation object.";
        proj_context_destroy(C);
        return {0.0, 0.0};
    }

    // 3. For EPSG:4326, PROJ expects Lat/Lon order.
    // If you prefer Lon/Lat output, you can enforce "axis mapping" like this:
    PJ *P_for_gis = proj_normalize_for_visualization(C, P);
    proj_destroy(P); // Free the original, use the normalized one

    // 4. Set up your input coordinates (X, Y)
    // proj_coord input order matches the normalized visualization: Longitude(X), Latitude(Y)
    PJ_COORD input = proj_coord(x, y, 0, 0);

    // 5. Do the transformation
    PJ_COORD output = proj_trans(P_for_gis, PJ_FWD, input);

    // 6. Clean up
    proj_destroy(P_for_gis);
    proj_context_destroy(C);

    // output.xy.x is Longitude, output.xy.y is Latitude
    return { output.xy.y, output.xy.x };
}
