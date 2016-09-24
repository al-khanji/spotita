import QtQuick 1.0

Rectangle {
    id: canvas
    width: 480
    height: 640
    color: "darkgreen"

    signal log_in(string username, string password)
    signal search(string query)
    signal quit()

    Connections {
        target: spotify_session
        onLoginSuccess: canvas.state = "logged_in"
        onLoginFailure: login_error_text.text = error_message
        onFatalError: {
            canvas.state = "error"
            error_message_text.text = error
        }
    }

    Rectangle {
            id: error_dialog
            z: 5
            anchors.fill: parent
            color: "red"
            enabled: false

            MouseArea {
                id: mouse_area3
                anchors.fill: parent
                onClicked: canvas.quit()
            }

            Text {
                id: fatal_error_text
                text: "FATAL ERROR"
                anchors.bottom: error_message_text.top
                anchors.bottomMargin: 5
                anchors.horizontalCenter: parent.horizontalCenter
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
            }

            Text {
                id: error_message_text
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
            }

            Text {
                id: instruction_text
                text: "Tap to exit"
                anchors.fill: parent
                verticalAlignment: Text.AlignBottom
                horizontalAlignment: Text.AlignHCenter
                anchors.bottomMargin: 50
            }

            Image {
                id: bomb_image
                x: 138
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.topMargin: 50
                source: "crash.png"
            }
    }

    Rectangle {
        id: log_in_page
        color: "#7c3030"
        z: 20
        anchors.fill: parent

        TextInput {
            id: username_input
            width: 80
            height: 20
            text: "textInput"
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.top: parent.top
            anchors.topMargin: 10
            font.pixelSize: 12
        }

        TextInput {
            id: password_input
            width: 80
            height: 20
            text: "textInput"
            echoMode: TextInput.Password
            anchors.leftMargin: 10
            anchors.left: parent.left
            anchors.top: username_input.bottom
            anchors.topMargin: 10
            font.pixelSize: 12
        }

        Rectangle {
            id: go_button
            width: 200
            height: 200
            color: "#ffffff"
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.top: password_input.bottom
            anchors.topMargin: 10

            MouseArea {
                id: mouse_area1
                anchors.fill: parent
                onClicked: canvas.log_in(username_input.text, password_input.text)
            }

            Text {
                id: text1
                text: "Log In"
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                anchors.fill: parent
                font.pixelSize: 12
            }
        }

        Text {
            id: login_error_text
            x: 221
            y: 48
            text: "text"
            font.pixelSize: 12
        }
    }

    Rectangle {
        id: logged_in_page
        color: "pink"
        anchors.fill: parent
        z: 1

        TextInput {
            id: search_input
            width: 96
            height: 20
            text: "textInput"
            anchors.leftMargin: 18
            anchors.topMargin: 18
            anchors.top: parent.top
            anchors.left: parent.left
            font.pixelSize: 12
        }

        Rectangle {
            id: search_button
            width: 114
            height: 20
            color: "#ffffff"
            anchors.topMargin: 18
            anchors.leftMargin: 10
            anchors.top: parent.top
            anchors.left: search_input.right

            Text {
                id: search_text
                text: "search"
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                anchors.fill: parent
                font.pixelSize: 12
            }

            MouseArea {
                id: mouse_area2
                anchors.fill: parent
                onClicked: canvas.search(search_input.text)
            }
        }

        ListView {
            id: track_result_listview
            height: parent.height / 3 - 40
            anchors.topMargin: 10
            anchors.top: search_input.bottom
            anchors.right: parent.right
            anchors.rightMargin: 18
            anchors.left: parent.left
            anchors.leftMargin: 18
            model: track_results
            delegate: Text {
                text: trackName + " by " + trackArtist
                MouseArea {
                    anchors.fill: parent
                    onClicked: spotify_session.playSearchTrack(index)
                }
            }
        }

        ListView {
            id: album_result_listview
            height: parent.height / 3 - 40
            anchors.right: parent.right
            anchors.rightMargin: 18
            anchors.left: parent.left
            anchors.leftMargin: 18
            anchors.topMargin: 10
            anchors.top: track_result_listview.bottom
            model: album_results
            delegate:  Item {
                height: 72
                Row {
                    Image {
                        asynchronous: true
                        source: albumCover
                        width: 64
                        height: 64
                    }

                    Text {
                        text: albumName + " by " + albumArtist
                    }
                }
            }
        }

        ListView {
            id: artistResultView
            height: parent.height / 3 - 40
            anchors.top: album_result_listview.bottom
            anchors.topMargin: 10
            anchors.right: parent.right
            anchors.rightMargin: 18
            anchors.left: parent.left
            anchors.leftMargin: 18
            model: artist_results
            delegate:  Text {
                text: artistName
            }
        }
    }

    states: [
        State {
            name: "logged_in"
            PropertyChanges {
                target: logged_in_page
                z: 20
            }
            PropertyChanges {
                target: log_in_page
                z: 5
            }
            PropertyChanges {
                target: error_dialog
                z: 5
            }
        },
        State {
            name: "error"
            PropertyChanges {
                target: logged_in_page
                z: 5
            }
            PropertyChanges {
                target: log_in_page
                z: 5
            }
            PropertyChanges {
                target: error_dialog
                enabled: true
                z: 20
            }
        }
    ]
}
