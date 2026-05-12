#include "DataExporter.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>

#include "DatabaseManager.h"

DataExporter::DataExporter(QObject* parent)
    : QObject(parent)
{
}

void DataExporter::setError(const QString& msg)
{
    m_lastError = msg;
    qWarning("DataExporter: %s", qUtf8Printable(msg));
    emit exportFailed(msg);
}

bool DataExporter::exportToFile(const QString& filePath)
{
    m_lastError.clear();

    DatabaseManager* dbm = DatabaseManager::instance();
    QList<EntityData> entities = dbm->fetchAllEntities();

    QJsonArray entityArray;

    for (const EntityData& e : entities) {
        QJsonObject entityObj;
        entityObj.insert(QStringLiteral("version"), 1);
        entityObj.insert(QStringLiteral("type"),    e.type.toLower());
        entityObj.insert(QStringLiteral("name"),    e.name);

        QJsonArray unitArray;
        QList<UnitData> units = dbm->getUnitsForParent(e.id);
        for (const UnitData& u : units) {
            QJsonObject unitObj;
            unitObj.insert(QStringLiteral("name"), u.name);

            QJsonArray sessionArray;
            QList<SessionTaskData> sessions = dbm->getSessionTasksForUnit(u.id);
            for (const SessionTaskData& s : sessions) {
                QJsonObject sObj;
                sObj.insert(QStringLiteral("name"),     s.name);
                sObj.insert(QStringLiteral("progress"), s.progress);
                sessionArray.append(sObj);
            }
            unitObj.insert(QStringLiteral("sessions"), sessionArray);
            unitArray.append(unitObj);
        }
        entityObj.insert(QStringLiteral("units"), unitArray);
        entityArray.append(entityObj);
    }

    QJsonObject root;
    root.insert(QStringLiteral("version"),  1);
    root.insert(QStringLiteral("entities"), entityArray);

    // Use QSaveFile so a crash mid-write doesn't corrupt the destination.
    QSaveFile out(filePath);
    if (!out.open(QIODevice::WriteOnly | QIODevice::Text)) {
        setError(QStringLiteral("Could not open file for writing: %1").arg(filePath));
        return false;
    }
    out.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    if (!out.commit()) {
        setError(QStringLiteral("Failed to flush file: %1").arg(filePath));
        return false;
    }

    emit exportCompleted(filePath);
    return true;
}
