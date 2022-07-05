#include <QFile>
#include <QRect>
#include <QThread>
#include <QTextStream>

#include "FlightDataModel.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <future>

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
    return 4;
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
        case 3:
            return QVariant(m_model[index.row()].m_altitude);
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
                    values[2].trimmed().toDouble(),
                    values[6].trimmed().toDouble()});
        }

        endResetModel();

        inputFile.close();

        setStatus("CSV file loaded, Model updated");
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
    std::sort(m_model.begin(), m_model.end(), FlightDataRowTimeComparator());
    m_sorted = true;
    endResetModel();

    setStatus("Model data sorted");
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
                    << "," << e.m_longitude 
                    << "," << e.m_altitude << ");";

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

    CreateJsonFromModel(m_model, "time_sorted.json");
}

QString FlightDataModel::status() const
{
    return m_status;
}

void FlightDataModel::setStatus(const QString& str)
{
    if (m_status != str)
    {
    m_status = str;
    emit statusChanged();
    }
}

QString FlightDataModel::DocToJsonString(rapidjson::Document& rjd)
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    rjd.Accept(writer);
    return buffer.GetString();
}

void FlightDataModel::multiThreadedSort()
{
   auto t1 = std::async(std::launch::async, &FlightDataModel::SortAltitude, this);
   auto t2 = std::async(std::launch::async, &FlightDataModel::FilterWesternHemisphere, this);
}

void FlightDataModel::SortAltitude(void)
{
   auto model = m_model;
   std::sort(model.begin(), model.end(), FlightDataRowAltitudeComparator());
   CreateJsonFromModel(model, "altitude_sorted.json");
}

void FlightDataModel::FilterWesternHemisphere(void)
{
   QVector<FlightDataRow> western;
   for (const auto& e : m_model)
   {
      if (e.m_longitude < 0)
        western.push_back(e);
   }
   CreateJsonFromModel(western, "western.json");
}

void FlightDataModel::CreateJsonFromModel(const QVector<FlightDataRow>& model, QString fileName)
{
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

    for (const auto& e : model)
    {
        rapidjson::Value coordinate(rapidjson::kArrayType);
        coordinate.PushBack(e.m_longitude, rjd.GetAllocator());
        coordinate.PushBack(e.m_latitude, rjd.GetAllocator());
        coordinate.PushBack(e.m_altitude, rjd.GetAllocator());
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
    file.open(fileName.toStdString().c_str());
    file << geoJson.toStdString();
    file.close();
}