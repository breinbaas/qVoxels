import QtQuick
import QtQuick3D
import QtQuick3D.AssetUtils
import QtQuick3D.Helpers

Item {
    id: root
    anchors.fill: parent
    property url modelSource: ""

    property real _modelMaxDim: 570
    property vector3d _modelCenter: Qt.vector3d(0, 0, 0)

    function focusOnModel() {
        orbitNode.eulerRotation.x = -20
        orbitNode.eulerRotation.y = 30
        flyAnimation.running = true
    }

    NumberAnimation {
        id: flyAnimation
        target: cameraArm
        property: "z"
        to: root._modelMaxDim * 2.0
        duration: 700
        easing.type: Easing.OutCubic
    }

    View3D {
        id: view
        anchors.fill: parent

        camera: camera

        environment: SceneEnvironment {
            clearColor: "#1a1a2e"
            backgroundMode: SceneEnvironment.Color
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.High
        }

        // Orbit rig: World → orbitNode (yaw+pitch pivot) → cameraArm (zoom) → camera
        // The camera always points toward the pivot naturally — no lookAt() needed.
        Node {
            id: orbitNode
            eulerRotation.x: -20
            eulerRotation.y: 30

            Node {
                id: cameraArm
                z: root._modelMaxDim * 2.0   // initial distance, updated on load

                PerspectiveCamera {
                    id: camera
                    // clipNear/Far scale with model size to prevent clipping
                    clipNear: root._modelMaxDim * 0.001
                    clipFar:  root._modelMaxDim * 2000
                }
            }
        }

        DirectionalLight {
            eulerRotation.x: -40
            eulerRotation.y: 20
            brightness: 1.5
        }
        DirectionalLight {
            eulerRotation.x: 30
            eulerRotation.y: 135
            brightness: 0.6
        }

        RuntimeLoader {
            id: glbLoader
            source: root.modelSource

            onStatusChanged: {
                if (status === RuntimeLoader.Success) {
                    var mn = glbLoader.bounds.minimum
                    var mx = glbLoader.bounds.maximum

                    var sx = mx.x - mn.x
                    var sy = mx.y - mn.y
                    var sz = mx.z - mn.z
                    var maxDim = Math.max(sx, Math.max(sy, sz))
                    if (maxDim <= 0) maxDim = 100

                    root._modelMaxDim = maxDim
                    cameraArm.z = maxDim * 2.0

                    statusLabel.text = "Loaded ✓  |  Size: "
                        + sx.toFixed(1) + " × " + sy.toFixed(1) + " × " + sz.toFixed(1)
                    console.log("Bounds min:", mn, "max:", mx, "maxDim:", maxDim)
                } else if (status === RuntimeLoader.Error) {
                    statusLabel.text = "Error: " + glbLoader.errorString
                    console.log("Load error:", glbLoader.errorString)
                }
            }
        }
    }

    // ── Mouse/touch input ────────────────────────────────────────────────────
    MouseArea {
        id: inputArea
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.MiddleButton | Qt.RightButton

        property real lastX: 0
        property real lastY: 0
        property int  activeButton: Qt.NoButton

        onPressed: (mouse) => {
            lastX = mouse.x
            lastY = mouse.y
            activeButton = mouse.button
        }

        onPositionChanged: (mouse) => {
            var dx = mouse.x - lastX
            var dy = mouse.y - lastY
            lastX = mouse.x
            lastY = mouse.y

            if (activeButton === Qt.LeftButton) {
                // Orbit
                orbitNode.eulerRotation.y += dx * 0.4
                var pitch = orbitNode.eulerRotation.x + dy * 0.4
                orbitNode.eulerRotation.x = Math.max(-89, Math.min(89, pitch))

            } else if (activeButton === Qt.MiddleButton || activeButton === Qt.RightButton) {
                // Pan: move the orbitNode in the camera's view-plane
                var panSpeed = cameraArm.z * 0.0015
                var yawRad = orbitNode.eulerRotation.y * Math.PI / 180.0

                // Right vector (camera local X) in world XZ
                orbitNode.x -= dx * panSpeed * Math.cos(yawRad)
                orbitNode.z -= dx * panSpeed * Math.sin(yawRad) * (-1)
                orbitNode.y += dy * panSpeed
            }
        }

        onWheel: (wheel) => {
            var factor = (wheel.angleDelta.y > 0) ? 0.88 : 1.14
            var newZ = cameraArm.z * factor
            cameraArm.z = Math.max(root._modelMaxDim * 0.01,
                           Math.min(root._modelMaxDim * 50, newZ))
        }
    }

    // ── HUD ──────────────────────────────────────────────────────────────────
    Rectangle {
        anchors.bottom: parent.bottom
        anchors.left:   parent.left
        anchors.right:  parent.right
        anchors.margins: 12
        height: 44
        radius: 8
        color: "#aa000000"

        Row {
            anchors.centerIn: parent
            spacing: 16

            Text {
                id: statusLabel
                text: "No model loaded"
                color: "#ccffffff"
                font.pixelSize: 12
                anchors.verticalCenter: parent.verticalCenter
            }

            Rectangle { width:1; height:24; color:"#44ffffff"; anchors.verticalCenter: parent.verticalCenter }

            Text {
                text: "Drag: Orbit  |  Right-drag / Middle-drag: Pan  |  Scroll: Zoom"
                color: "#88ffffff"
                font.pixelSize: 11
                anchors.verticalCenter: parent.verticalCenter
            }

            Rectangle { width:1; height:24; color:"#44ffffff"; anchors.verticalCenter: parent.verticalCenter }

            Rectangle {
                width: 88; height: 28; radius: 6
                color: flyBtn.containsMouse ? "#6666bbff" : "#33ffffff"
                anchors.verticalCenter: parent.verticalCenter
                Behavior on color { ColorAnimation { duration: 120 } }
                Text { anchors.centerIn: parent; text: "⌖  Fly to model"; color: "white"; font.pixelSize: 11 }
                MouseArea { id: flyBtn; anchors.fill: parent; hoverEnabled: true; onClicked: root.focusOnModel() }
            }
        }
    }
}
