import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import AiassistAsync 1.0

Rectangle {
    color: "red"
    width: parent.width
    height: parent.height
    
    Rectangle {
        id: bg
        color: "#212121"
        anchors.fill: parent
    }

    //property string fullText: "Hi Stefan. Welcome to your digital automotive space."
    property string fullText: ""
    property string textDisplay: ""
    property int currentIndex: 0

    //Component.onCompleted: {
    //    typingTimer.start()
    //}

    AiassistAsync {
        id: aiassistAsync

        onUpdateTextToSpeech: (msg)=> {
            //console.log("tts: ", msg)
            textDisplay = ""
            fullText = ""
            fullText = msg
            currentIndex = 0
            typingTimer.stop()
            typingTimer.start()
        }

        onSetSecurityIsAttacked: (sts) => {
            if (sts == true) {
                sec_rect.visible = true
                sec_targetColor = "red"
                sec_colorAnimation.running = true
            } else {
                sec_rect.visible = false                
                sec_colorAnimation.running = false
            }
        }

        onSetSecurityReactionStage: (secReact) => {
            if (secReact == 0) {                
                sec_targetColor = "red"
                sec_colorAnimation.running = true
            }
            if (secReact == 1) {
                // sec_security_processing
                sec_rect.visible = true
                sec_targetColor = "blue"
                sec_colorAnimation.running = true
            }
            if (secReact == 2) {
                // sec_car_safe
                sec_rect.visible = true
                sec_rect.color = "green"
                sec_colorAnimation.running = false
            }
            if (secReact == 3) {                

            }
        }
    }

    property int iconSize: 100
    property int iconSpacing: 300

    // First logo
    Image {
        id: bgswlogo
        y: 20
        x: 40
        source: "resource/bgswlogo.png"
        width: 80
        height: 80
        fillMode: Image.PreserveAspectFit
    }
    // Second logo
    Image {
        id: digitalautologo
        y: 0
        x: bgswlogo.x + iconSpacing - 20
        source: "resource/logo2.png" 
        width: 130
        height: 130
        fillMode: Image.PreserveAspectFit
    }
    // Third logo
    Image {
        id: etaslogo
        y: -35
        x: digitalautologo.x + iconSpacing
        //source: "resource/etaslogo.jpg" 
        source: "resource/logo3.png" 
        width: 200
        height: 200
        fillMode: Image.PreserveAspectFit
    }
    // Fourth logo
    Image {
        id: boschlogo
        y: -85
        x: etaslogo.x + iconSpacing
        //source: "resource/boschlogo.png"
        source: "resource/logo4.png" 
        width: 300  
        height: 300
        fillMode: Image.PreserveAspectFit
    }

    property string sec_defaultColor: "#212121"
    property string sec_targetColor: "red"
    Rectangle {
        id: sec_rect
        width: parent.width
        height: parent.height
        color: "blue"
        opacity: 0.5
        visible: false

        // Define the color animation
        SequentialAnimation on color {
            id: sec_colorAnimation
            loops: Animation.Infinite // Loop the animation infinitely
            running: false // Start with the animation stopped
            ColorAnimation {
                from: sec_defaultColor
                to: sec_targetColor
                duration: 1000 // 1 second
            }
            ColorAnimation {
                from: sec_targetColor
                to: sec_defaultColor
                duration: 1000 // 1 second
            }
        }
    }

    Image {
        id: pendulum
        source: "resource/bot.png"
        width: 700*2/3
        height: 825*2/3
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        transformOrigin: Item.Center
        // Pendulum swing animation
        SequentialAnimation on rotation {
            loops: Animation.Infinite
            PropertyAnimation {
                from: -5
                to: 5
                duration: 1000
                easing.type: Easing.InOutQuad
            }
            PropertyAnimation {
                from: 5
                to: -5
                duration: 1000
                easing.type: Easing.InOutQuad
            }
        }

        MouseArea {
                anchors.fill: parent
                onClicked: {
                    mainWindow.visibility = Window.FullScreen
                    //console.log("Logo 1 clicked")
                    //aiassistAsync.setTextToSpeech("Hello Stefan. Welcome to your digital automotive space. Your custom coffee is being prepared. Fasten seat belt, relax, and enjoy your exceptional journey")
                }
            }
    }

    Rectangle {
        id: textContainer
        width: parent.width * 0.6
        height: 50
        //x: 450
        y: pendulum.y + pendulum.height + 10
        anchors.horizontalCenter: parent.horizontalCenter
        //anchors.verticalCenter: parent.verticalCenter
        //anchors.bottom: parent.bottom
        //anchors.bottomMargin: 20
        radius: 10
        color: "transparent"
        //color: "#333"
        //border.color: "#777"
        //border.width: 2
        //opacity: 0.8

        Text {
            id: text2speechField
            width: parent.width - 20
            height: parent.height
            anchors.centerIn: parent
            font.pixelSize: 30
            font.bold: true
            text: textDisplay
            color: "white"
            wrapMode: TextEdit.Wrap 
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            //lineHeight: 6
            lineHeight: 1.1
            lineHeightMode: Text.ProportionalHeight
        }
    }

    Timer {
        id: typingTimer
        interval: 50 // Adjust typing speed (in milliseconds)
        repeat: true
        onTriggered: {
            if (currentIndex < fullText.length) {
                textDisplay += fullText.charAt(currentIndex)
                currentIndex++
            } else {
                currentIndex++
                if (currentIndex > (fullText.length + 20)) {
                    typingTimer.stop()
                    textDisplay = ""
                    currentIndex = 0
                }                
            }
        }
    }
}
