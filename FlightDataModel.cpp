#include <QFile>
#include <QRect>
#include <QTextStream>

#include "FlightDataModel.h"

#include <iostream>
#include <sstream>
#include <fstream>

extern sqlite3 *db;
int callback(void *NotUsed, int argc, char **argv, char **azColName) {
   int i;
   for(i = 0; i<argc; i++) {
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   return 0;
}

FlightDataModel::FlightDataModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    clear();
}

int FlightDataModel::rowCount(const QModelIndex &parent) const
{
    return m_model.size();
}

int FlightDataModel::columnCount(const QModelIndex &parent) const
{
    return 3;
}

QVariant FlightDataModel::data(const QModelIndex &index, int role) const
{
    switch (index.column())
    {
        case 0:
            return QVariant(m_model[index.row()].m_time.toString());
        case 1:
            return QVariant(m_model[index.row()].m_latitude);
        case 2:
            return QVariant(m_model[index.row()].m_longitude);
        default:
            return QVariant();
    }
}

void FlightDataModel::clear()
{
    int n = m_model.size();
    m_model.clear();
    emit dataChanged(index(0, 0), index(n - 1, 3 - 1));
}

void FlightDataModel::loadCSV()
{
    QFile inputFile("flightdata.csv");
    if (inputFile.open(QIODevice::ReadOnly))
    {
        beginResetModel();
        QTextStream in(&inputFile);
        while (!in.atEnd())
        {
            QString line = in.readLine();

            QStringList values = line.split(",");

            auto time = QTime::fromString(values[0].trimmed(), "h:mm:ss AP");

            if(time.isValid())
                m_model.push_back({
                    time,
                    values[1].trimmed().toDouble(),
                    values[2].trimmed().toDouble()});
        }

        inputFile.close();

        setStatus("CSV file loaded, Model updated");

        endResetModel();
    }
}

void FlightDataModel::sortModel()
{
    if (!m_model.size())
    {
        setStatus("Please load the csv data first.");
        return;
    }

    beginResetModel();
    std::sort(m_model.begin(), m_model.end(), FlightDataRowComparator());
    m_sorted = true;
    setStatus("Model data sorted");
    endResetModel();
}

void FlightDataModel::writeToDB()
{
    if (!m_model.size())
    {
      setStatus("Please load the csv data first.");
      return;
    }

    if (!m_sorted)
    {
      sortModel();
    }

    setStatus("Writing sorted model data to DB ...");

    for (const auto& e : m_model)
    {
        std::ostringstream ss;
        ss << "INSERT INTO FLIGHTTABLE VALUES ";
        ss << "(NULL, \"" << e.m_time.toString("hh:mm:ss.zzz").toStdString() << "\""
                    << "," << e.m_latitude 
                    << "," << e.m_longitude << ");";

        qDebug() << QString(ss.str().c_str());
        char *zErrMsg = 0;
        int rc = sqlite3_exec(db, ss.str().c_str(), callback, 0, &zErrMsg);
        
        if( rc != SQLITE_OK ) {
            qDebug() << "SQL error: " << QString(zErrMsg);
            sqlite3_free(zErrMsg);
        }
    }

    setStatus("Sorted model data written to DB");
}

void FlightDataModel::writeGeoJSON()
{
    if (!m_model.size())
    {
      setStatus("Please load the csv data first.");
      return;
    }

    if (!m_sorted)
    {
      sortModel();
    }

    rapidjson::Document rjd;
    rjd.SetObject();

    rjd.AddMember("type", "FeatureCollection", rjd.GetAllocator());

    rapidjson::Value features; // array
    features.SetArray();

    rapidjson::Value fo; // has type and geometry
    fo.SetObject();

    fo.AddMember("type", "Feature", rjd.GetAllocator());

    rapidjson::Value geometry;
    geometry.SetObject();

    geometry.AddMember("type", "LineString", rjd.GetAllocator());

    rapidjson::Value coordinatesArray(rapidjson::kArrayType);

    for (const auto& e : m_model)
    {
        rapidjson::Value coordinate(rapidjson::kArrayType);
        coordinate.PushBack(e.m_longitude, rjd.GetAllocator());
        coordinate.PushBack(e.m_latitude, rjd.GetAllocator());
        coordinatesArray.PushBack(coordinate, rjd.GetAllocator());
    }

    geometry.AddMember("coordinates", coordinatesArray, rjd.GetAllocator());

    fo.AddMember("geometry", geometry, rjd.GetAllocator());

    // add properties object here
    rapidjson::Value properties;
    properties.SetObject();

    properties.AddMember("test", "xxx", rjd.GetAllocator());

    fo.AddMember("properties", properties, rjd.GetAllocator());

    features.PushBack(fo, rjd.GetAllocator());

    rjd.AddMember("features", features, rjd.GetAllocator());

    auto geoJson = DocToJsonString(rjd);

    qDebug() << geoJson;
    
    std::ofstream file;
    file.open ("geo.json");
    file << geoJson.toStdString();
    file.close();
}

QString FlightDataModel::DocToJsonString(rapidjson::Document& rjd)
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    rjd.Accept(writer);
    return buffer.GetString();
}