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

        PluginParameter {
            name: "osm.mapping.custom.host"
            value: "https://tile.openstreetmap.org/"
        }
    }

    Map {
        id: map
        anchors.fill: parent
        plugin: mapPlugin
        center: QtPositioning.coordinate(52.5200, 13.4050)
        zoomLevel: 13
        activeMapType: map.supportedMapTypes[map.supportedMapTypes.length - 1]


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
        MapItemView {
            model: cptManager.cptList

            delegate: MapItemGroup {
                // Enclosing everything inside a group fixes the "child items" error

                MapCircle {
                    // Access properties natively via modelData or model (depending on your model type)
                    center: QtPositioning.coordinate(modelData.latitude, modelData.longitude)
                    radius: 5 // Radius in meters
                    color: "black"
                    border.color: "darkred"
                    border.width: 1
                }

                MapQuickItem {
                    coordinate: QtPositioning.coordinate(modelData.latitude, modelData.longitude)
                    anchorPoint.x: sourceItem.width / 2  // Center horizontally
                    anchorPoint.y: sourceItem.height + 7 // Position just above the dot

                    sourceItem: Text {
                        text: modelData.name || "CPT"
                        font.pixelSize: 15
                        color: "black"

                        // Optional styling to make text legible against a map
                        style: Text.Outline
                        styleColor: "white"

                        visible: map.zoomLevel > 14 // Only show labels at higher zoom levels
                    }
                }
            }
        }
        Connections {
            target: cptManager
            onCptListChanged: {
                if (cptManager.cptList.length > 0) {
                    map.fitViewportToMapItems();
                }
            }
        }

    }
}