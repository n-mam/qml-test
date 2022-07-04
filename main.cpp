#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "FlightDataModel.h"
#include <iostream>

sqlite3 *db = nullptr;

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
  QGuiApplication app(argc, argv);

  QQmlApplicationEngine engine;
  
  const QUrl url(QStringLiteral("qrc:/main.qml"));

  QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                   &app, [url](QObject *obj, const QUrl &objUrl) {
    if (!obj && url == objUrl)
      QCoreApplication::exit(-1);
  }, Qt::QueuedConnection);

  qmlRegisterType<FlightDataModel>("com.mycompany.qmlcomponents",1,0, "FlightDataModel");

  char *zErrMsg = 0;
  int rc;

  rc = sqlite3_open("data.db", &db);

  if( rc ) {
    std::cout << "cant open db" << std::endl;
    return(0);
  } else {
    std::cout << "opened db successfully" << std::endl;
  }

  const char *sql = "CREATE TABLE FLIGHTTABLE("  \
    "ID             INTEGER PRIMARY KEY AUTOINCREMENT," \
    "TIME           TEXT    NOT NULL," \
    "LATITUDE       REAL    NOT NULL," \
    "LONGITUDE      REAL    NOT NULL);";

  rc = sqlite3_exec(db, sql, NULL, 0, &zErrMsg);
  
  if( rc != SQLITE_OK ) {
    qDebug() << "SQL error : " << QString(zErrMsg);
    sqlite3_free(zErrMsg);
  } else {
    qDebug() << "Table created successfully";
  }

  engine.load(url);

  app.exec();

  sqlite3_close(db);

  return 0;
}
