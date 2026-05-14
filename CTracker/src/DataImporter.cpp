#include "DataImporter.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include <QtGlobal>

#include "DatabaseManager.h"

DataImporter::DataImporter(QObject* parent)
    : QObject(parent)
{
}

void DataImporter::setError(const QString& msg)
{
    m_lastError = msg;
    qWarning("DataImporter: %s", qUtf8Printable(msg));
    emit importFailed(msg);
}

bool DataImporter::importFromFile(const QString& filePath)
{
    m_lastError.clear();

    // ---- 1. Open + parse ----
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        setError(QStringLiteral("Could not open file: %1").arg(filePath));
        return false;
    }

    QJsonParseError parseErr;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseErr);
    file.close();

    if (parseErr.error != QJsonParseError::NoError) {
        setError(QStringLiteral("Invalid JSON: %1").arg(parseErr.errorString()));
        return false;
    }
    if (!doc.isObject()) {
        setError(QStringLiteral("JSON root must be an object."));
        return false;
    }

    QJsonObject root = doc.object();

    // ---- 2. Validate required fields ----
    if (!root.contains(QStringLiteral("version")) ||
        !root.contains(QStringLiteral("type"))    ||
        !root.contains(QStringLiteral("name"))    ||
        !root.contains(QStringLiteral("units"))) {
        setError(QStringLiteral("Missing one of: version, type, name, units."));
        return false;
    }

    const QString type = root.value(QStringLiteral("type")).toString().toLower();
    const QString name = root.value(QStringLiteral("name")).toString().trimmed();
    const QJsonValue unitsVal = root.value(QStringLiteral("units"));

    if (type != QStringLiteral("course") && type != QStringLiteral("project")) {
        setError(QStringLiteral("type must be \"course\" or \"project\" (got \"%1\").").arg(type));
        return false;
    }
    if (name.isEmpty()) {
        setError(QStringLiteral("Entity name is empty."));
        return false;
    }
    if (!unitsVal.isArray()) {
        setError(QStringLiteral("\"units\" must be an array."));
        return false;
    }

    // ---- 3. One transaction for the whole import ----
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.transaction()) {
        setError(QStringLiteral("Could not begin transaction: %1")
                     .arg(db.lastError().text()));
        return false;
    }

    DatabaseManager* dbm = DatabaseManager::instance();

    int entityId = (type == QStringLiteral("course"))
                       ? dbm->addCourse(name)
                       : dbm->addProject(name);

    if (entityId <= 0) {
        db.rollback();
        setError(QStringLiteral("Failed to create entity \"%1\".").arg(name));
        return false;
    }

    // ---- 4. Walk units → sessions ----
    const QJsonArray units = unitsVal.toArray();
    for (const QJsonValue& uv : units) {
        if (!uv.isObject()) {
            qWarning("DataImporter: skipping non-object unit entry");
            continue;
        }
        const QJsonObject uo = uv.toObject();
        const QString unitName = uo.value(QStringLiteral("name")).toString().trimmed();
        if (unitName.isEmpty()) {
            qWarning("DataImporter: skipping unit with empty name");
            continue;
        }

        int unitId = dbm->addUnit(entityId, unitName);
        if (unitId <= 0) {
            qWarning("DataImporter: failed to add unit \"%s\" — skipping its sessions",
                     qUtf8Printable(unitName));
            continue;
        }

        const QJsonValue sessionsVal = uo.value(QStringLiteral("sessions"));
        if (!sessionsVal.isArray()) continue;

        const QJsonArray sessions = sessionsVal.toArray();
        for (const QJsonValue& sv : sessions) {
            if (!sv.isObject()) {
                qWarning("DataImporter: skipping non-object session entry");
                continue;
            }
            const QJsonObject so = sv.toObject();
            const QString sName = so.value(QStringLiteral("name")).toString().trimmed();
            if (sName.isEmpty()) {
                qWarning("DataImporter: skipping session with empty name");
                continue;
            }

            int progress = so.value(QStringLiteral("progress")).toInt(0);
            progress = qBound(0, progress, 100);   // clamp to [0,100]

            if (dbm->addSessionTask(unitId, sName, progress) <= 0) {
                qWarning("DataImporter: failed to add session \"%s\"",
                         qUtf8Printable(sName));
            }
        }
    }

    if (!db.commit()) {
        db.rollback();
        setError(QStringLiteral("Commit failed: %1").arg(db.lastError().text()));
        return false;
    }

    emit importCompleted(entityId);
    return true;
}
