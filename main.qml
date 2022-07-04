import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls
import com.mycompany.qmlcomponents 1.0

Window {
    id: window
    width: 450
    height: 480
    visible: true
    title: qsTr("Hello World")

    Button {
        id: buttonLoadCSV
        x: 14
        y: 430
        width: 97
        height: 24
        text: qsTr("Load CSV")
        onClicked: {
            flightModel.loadCSV();
        }
    }

    Button {
        id: buttonSortData
        x: 127
        y: 430
        width: 97
        height: 24
        text: qsTr("Sort Data")
        onClicked: {
          flightModel.sortModel();
        }
    }

    Button {
        id: buttonWriteJSON
        x: 236
        y: 430
        width: 97
        height: 24
        text: qsTr("Write JSON")
        onClicked: {
          flightModel.writeGeoJSON();
        }
    }

    Button {
        id: buttonWriteDB
        x: 341
        y: 430
        width: 97
        height: 24
        text: qsTr("Write To DB")
        onClicked: {
          flightModel.writeToDB();
        }
    }

    Rectangle {
        id: rectangle
        x: 14
        y: 13
        width: 424
        height: 396
        color: "#ffffff"
        radius: 2
        border.color: "#434343"
        border.width: 1

        TableView {
            id: tableView
            x: 12
            y: 13
            width: 400
            height: 354

            ScrollBar.horizontal: ScrollBar {}
            ScrollBar.vertical: ScrollBar {}

            model: FlightDataModel {
                id: flightModel
            }

            delegate: Rectangle {
                implicitWidth: 100
                implicitHeight: 25
                Text {
                    text: display
                }
            }
        }
    }

    Text {
        text: flightModel.status
        x: 14
        y: 460
    }
}
