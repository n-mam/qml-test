import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls
import com.mycompany.qmlcomponents 1.0

Window {
    id: window
    width: 550
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
        text: qsTr("Write Geo Json")
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

    Button {
        id: buttonMTSort
        x: 452
        y: 430
        width: 86
        height: 24
        text: qsTr("MT Sort")
        onClicked: {
          flightModel.multiThreadedSort();
        }
    }

    Rectangle {
        id: rectangle
        x: 14
        y: 13
        width: 524
        height: 396
        color: "#ffffff"
        radius: 2
        border.color: "#434343"
        border.width: 1

        TableView {
            id: tableView
            x: 12
            y: 13
            width: 500
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
