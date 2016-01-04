import QtQuick 2.5
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.0
import QtQuick.Window 2.2

ApplicationWindow {
    id: root
    width: Screen.width
    height: Screen.height
    title: "PiGaCo"

    ShaderEffect {
        width: parent.width
        height: parent.height

        property real time: 0
        property point resolution: Qt.point(parent.width, parent.height)

        NumberAnimation on time {
            loops: Animation.Infinite
            from: Math.random() * (400 - 0) + 0;
            to: 400
            duration: 600000
        }

        fragmentShader:         "precision mediump float;
                                uniform float     time;
                                uniform vec2    resolution;
                                #define PI 3.1415926535897932384626433832795
                                const float position = 0.0;
                                const float scale = 1.0;
                                const float intensity = 1.0;
                                float band(vec2 pos, float amplitude, float frequency) {
                                    float wave = scale * amplitude * sin(1.0 * PI * frequency * pos.x + time) / 2.05;
                                    float light = clamp(amplitude * frequency * 0.02, 0.001 + 0.001 / scale, 5.0) * scale / abs(wave - pos.y);
                                    return light;
                                }
                                void main() {
                                    vec3 color = vec3(1.5, 0.5, 10.0);
                                    color = color == vec3(0.0)? vec3(10.5, 0.5, 1.0) : color;
                                    vec2 pos = (gl_FragCoord.xy / resolution.xy);
                                    pos.y += - 0.5;
                                    float spectrum = 0.0;
                                    const float lim = 28.0;
                                    #define time time*0.037 + pos.x*10.
                                    for(float i = 0.0; i < lim; i++){
                                        spectrum += band(pos, 1.0*sin(time*0.1/PI), 1.0*sin(time*i/lim))/pow(lim, 0.25);
                                    }
                                    spectrum += band(pos, cos(10.7), 2.5);
                                    spectrum += band(pos, 0.4, sin(2.0));
                                    spectrum += band(pos, 0.05, 4.5);
                                    spectrum += band(pos, 0.1, 7.0);
                                    spectrum += band(pos, 0.1, 1.0);
                                    gl_FragColor = vec4(color * spectrum, spectrum);
                                }"
        //Thanks to the Phaser Lightwave example: http://phaser.io/examples/v2/filters/lightwave (MIT-License)
    }


    MainMenuHeader {
        id: header
        width: parent.width
        height: 120

        KeyNavigation.down: gameList

        onStoreClicked: {
            pageLoader.source = "Pages/Store/Main.qml"
            pageLoader.focus = true
        }
        onSettingsClicked: {
            pageLoader.source = "Pages/Settings/Main.qml"
            pageLoader.focus = true
        }
    }

    ListView {
        id: gameList
        x: 0
        y: parent.height / 4
        width: parent.width
        height: parent.height / 2
        orientation: ListView.Horizontal
        spacing: 20
        focus: true

        KeyNavigation.up: header

        model: dirScanner
        delegate: GameBox {
            width: 250
            height: parent.height
        }

    }
    Loader {
        id: pageLoader
        anchors.fill: parent

        onSourceChanged: {
            header.focus = true
        }

    }
}
