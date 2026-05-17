#include "core/DataImporter.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QSqlDatabase>
#include <QSqlError>
#include <QtGlobal>
#include <QVariant>

#include "core/DatabaseManager.h"

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

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        setError(QStringLiteral("Could not open file: %1").arg(filePath));
        return false;
    }

    QJsonParseError parseErr;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseErr);
    file.close();

    if (parseErr.error != QJsonParseError::NoError) {
        setError(QStringLiteral("Invalid JSON: %1").arg(parseErr.errorString()));
        return false;
    }
    if (!doc.isObject()) {
        setError(QStringLiteral("JSON root must be an object."));
        return false;
    }

    const QJsonObject root = doc.object();
    QJsonArray entities;
    if (root.contains(QStringLiteral("entities"))) {
        if (!root.value(QStringLiteral("entities")).isArray()) {
            setError(QStringLiteral("\"entities\" must be an array."));
            return false;
        }
        entities = root.value(QStringLiteral("entities")).toArray();
    } else {
        entities.append(root);
    }

    DatabaseManager* dbm = DatabaseManager::instance();
    QSqlDatabase db = dbm->database();
    if (!db.transaction()) {
        setError(QStringLiteral("Could not begin transaction: %1").arg(db.lastError().text()));
        return false;
    }

    int firstEntityId = -1;

    for (const QJsonValue& entityVal : entities) {
        if (!entityVal.isObject()) {
            db.rollback();
            setError(QStringLiteral("Each entity entry must be an object."));
            return false;
        }

        const QJsonObject entity = entityVal.toObject();
        if (!entity.contains(QStringLiteral("version")) ||
            !entity.contains(QStringLiteral("type")) ||
            !entity.contains(QStringLiteral("name")) ||
            !entity.contains(QStringLiteral("units"))) {
            db.rollback();
            setError(QStringLiteral("Missing one of: version, type, name, units."));
            return false;
        }

        const QString type = entity.value(QStringLiteral("type")).toString().toLower();
        const QString name = entity.value(QStringLiteral("name")).toString().trimmed();
        const QJsonValue unitsVal = entity.value(QStringLiteral("units"));

        if (type != QStringLiteral("course") && type != QStringLiteral("project")) {
            db.rollback();
            setError(QStringLiteral("type must be \"course\" or \"project\" (got \"%1\").").arg(type));
            return false;
        }
        if (name.isEmpty()) {
            db.rollback();
            setError(QStringLiteral("Entity name is empty."));
            return false;
        }
        if (!unitsVal.isArray()) {
            db.rollback();
            setError(QStringLiteral("\"units\" must be an array."));
            return false;
        }

        const int entityId = (type == QStringLiteral("course"))
            ? dbm->addCourse(name)
            : dbm->addProject(name);

        if (entityId <= 0) {
            db.rollback();
            setError(QStringLiteral("Failed to create entity \"%1\".").arg(name));
            return false;
        }
        if (firstEntityId < 0) {
            firstEntityId = entityId;
        }

        const QJsonArray units = unitsVal.toArray();
        for (const QJsonValue& unitVal : units) {
            if (!unitVal.isObject()) {
                qWarning("DataImporter: skipping non-object unit entry");
                continue;
            }

            const QJsonObject unit = unitVal.toObject();
            const QString unitName = unit.value(QStringLiteral("name")).toString().trimmed();
            if (unitName.isEmpty()) {
                qWarning("DataImporter: skipping unit with empty name");
                continue;
            }

            const int unitId = dbm->addUnit(entityId, unitName);
            if (unitId <= 0) {
                qWarning("DataImporter: failed to add unit \"%s\" - skipping its sessions",
                         qUtf8Printable(unitName));
                continue;
            }

            const QJsonValue sessionsVal = unit.value(QStringLiteral("sessions"));
            if (!sessionsVal.isArray()) {
                continue;
            }

            const QJsonArray sessions = sessionsVal.toArray();
            for (const QJsonValue& sessionVal : sessions) {
                if (!sessionVal.isObject()) {
                    qWarning("DataImporter: skipping non-object session entry");
                    continue;
                }

                const QJsonObject session = sessionVal.toObject();
                const QString sessionName = session.value(QStringLiteral("name")).toString().trimmed();
                if (sessionName.isEmpty()) {
                    qWarning("DataImporter: skipping session with empty name");
                    continue;
                }

                int progress = session.value(QStringLiteral("progress")).toInt(0);
                progress = qBound(0, progress, 100);

                if (dbm->addSessionTask(unitId, sessionName, progress) <= 0) {
                    qWarning("DataImporter: failed to add session \"%s\"",
                             qUtf8Printable(sessionName));
                }
            }
        }
    }

    if (!db.commit()) {
        db.rollback();
        setError(QStringLiteral("Commit failed: %1").arg(db.lastError().text()));
        return false;
    }

    emit importCompleted(firstEntityId);
    return true;
}
