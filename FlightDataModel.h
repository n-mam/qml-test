#ifndef FlightDataModel_H
#define FlightDataModel_H

#include <array>
#include <QPoint>
#include <QTime>
#include <QDebug>
#include <QtQml/qqml.h>
#include <QAbstractTableModel>

#include <sqlite3.h> 

struct FlightDataRow
{
  QTime m_time = {0,0,0};
  double m_latitude = 0;
  double m_longitude = 0;
  bool operator < (const FlightDataRow &rhs) const
  {
    return(this->m_time < rhs.m_time);
  }
};

struct FlightDataRowComparator
{
    bool operator()(const FlightDataRow &a, const FlightDataRow &b) 
    {
        return(a < b);
    }
};

class FlightDataModel : public QAbstractTableModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString status READ status WRITE setStatus NOTIFY statusChanged)

  public:

    QHash<int, QByteArray> roleNames() const override
    {
        return { {Qt::DisplayRole, "display"} };
    }

    explicit FlightDataModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Q_INVOKABLE void clear();
    Q_INVOKABLE void loadCSV();
    Q_INVOKABLE void sortModel();
    Q_INVOKABLE void writeToDB();
    Q_INVOKABLE void writeGeoJSON();

    void setStatus(const QString& str)
    {
      if (m_status != str)
      {
        m_status = str;
        emit statusChanged();
      }
    }
    
    QString status() const 
    {
      return m_status;
    }

  signals:

    void statusChanged();

  private:

    QVector<FlightDataRow> m_model;
    bool m_sorted = false;
    QString m_status = "Ready";
};

#endif // FlightDataModel_H
