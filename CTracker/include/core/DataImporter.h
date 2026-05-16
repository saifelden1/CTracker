#pragma once

#include <QObject>
#include <QString>

// ============================================================
//  DataImporter.h
//  used to import data from a json file for the ease of imporation of course using ai instead
//  of adding them manually
//
//  Reads a JSON file describing a Course or Project (with its
//  Units and Sessions/Tasks) and writes it into the database
//  inside ONE transaction. Either the whole import succeeds or
//  nothing is inserted.
//
//  Expected JSON shape:
//  {
//    "version": 1,
//    "type":    "course" | "project",
//    "name":    "ROS 2",
//    "units":   [
//      { "name": "Topics",   "sessions": [ { "name": "Intro", "progress": 50 } ] },
//      { "name": "Services", "sessions": [ ... ] }
//    ]
//  }
// ============================================================

class DataImporter : public QObject
{
    Q_OBJECT

public:
    explicit DataImporter(QObject* parent = nullptr);

    // Returns true on full success. On failure the transaction is
    // rolled back and no rows are written.
    bool importFromFile(const QString& filePath);

    // Last error message (human readable) for the most recent call.
    QString lastError() const { return m_lastError; }

signals:
    void importCompleted(int entityId);
    void importFailed(const QString& reason);

private:
    void setError(const QString& msg);
    QString m_lastError;
};
