import QtQuick
import QtPositioning
import QtLocation

Item {
    id: root
    visible: true
    width: 800
    height: 600

    Plugin {
        id: mapPlugin
        name: "osm"
    }

    Map {
        id: map
        anchors.fill: parent
        plugin: mapPlugin
        center: QtPositioning.coordinate(52.5200, 13.4050)
        zoomLevel: 13



        property bool selectionMode: false
        property var startCoord: null

        // --- MOUSE WHEEL ZOOMING ---
        WheelHandler {
            id: wheelHandler
            onWheel: (event) => {
                if (event.angleDelta.y > 0) {
                    if (map.zoomLevel < map.maximumZoomLevel) map.zoomLevel += 0.5
                } else {
                    if (map.zoomLevel > map.minimumZoomLevel) map.zoomLevel -= 0.5
                }
            }
        }

        DragHandler {
            enabled: !map.selectionMode

            onTranslationChanged: (delta) => {
                map.pan(-delta.x, -delta.y)
            }
        }

        // --- AREA SELECTION HANDLING ---
        MapRectangle {
            id: selectionBox
            color: "#330000FF"
            border.color: "blue"
            border.width: 2
            visible: false
        }

        MouseArea {
            anchors.fill: parent
            enabled: map.selectionMode
            preventStealing: true

            onPressed: (mouse) => {
                var coord = map.toCoordinate(Qt.point(mouse.x, mouse.y));
                map.startCoord = coord;
                selectionBox.topLeft = coord;
                selectionBox.bottomRight = coord;
                selectionBox.visible = true;
            }
            onPositionChanged: (mouse) => {
                if (!map.startCoord) return;
                var currentCoord = map.toCoordinate(Qt.point(mouse.x, mouse.y));
                selectionBox.topLeft = QtPositioning.coordinate(
                    Math.max(map.startCoord.latitude, currentCoord.latitude),
                    Math.min(map.startCoord.longitude, currentCoord.longitude)
                );
                selectionBox.bottomRight = QtPositioning.coordinate(
                    Math.min(map.startCoord.latitude, currentCoord.latitude),
                    Math.max(map.startCoord.longitude, currentCoord.longitude)
                );
            }
            onReleased: {
                if (map.startCoord) {
                    cptManager.reportSelectedArea(
                        selectionBox.bottomRight.latitude,
                        selectionBox.topLeft.longitude,
                        selectionBox.topLeft.latitude,
                        selectionBox.bottomRight.longitude
                    );
                    map.startCoord = null;
                }
            }
        }
    }
}