#pragma once

#include <QObject>
#include <QString>

// ============================================================
//  DataExporter.h
//
//  Walks the database and serialises every Course + Project (with
//  their Units and Sessions/Tasks) into a single JSON file on disk.
//
//  Output shape — top-level is an object with an "entities" array
//  so a future importer can read several at once:
//  {
//    "version":  1,
//    "entities": [
//      { "version": 1, "type": "course", "name": "ROS 2", "units": [...] },
//      { "version": 1, "type": "project", "name": "Robot Arm", "units": [...] }
//    ]
//  }
// ============================================================

class DataExporter : public QObject
{
    Q_OBJECT

public:
    explicit DataExporter(QObject* parent = nullptr);

    bool exportToFile(const QString& filePath);
    QString lastError() const { return m_lastError; }

signals:
    void exportCompleted(const QString& filePath);
    void exportFailed(const QString& reason);

private:
    void setError(const QString& msg);
    QString m_lastError;
};
