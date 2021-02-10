#include "jsonsessionloader.h"

#include <QFile>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

JsonSessionLoader::JsonSessionLoader(QObject *t_Parent) :
    QObject(t_Parent)
{
}

const QString &JsonSessionLoader::getSessionFilePath() const
{
    return m_currentSessionFile;
}

void JsonSessionLoader::loadSession(QString t_filePath)
{
    QFile sesFile;
    sesFile.setFileName(t_filePath);

    if(sesFile.exists() && sesFile.open(QIODevice::Unbuffered | QIODevice::ReadOnly)) {
        qInfo() << "loading session:" << t_filePath;
        QByteArray sesFileContent;
        QJsonDocument jsonSession;
        QJsonParseError jsonError;

        sesFileContent = sesFile.readAll();
        sesFile.close();

        jsonSession = QJsonDocument::fromJson(sesFileContent, &jsonError);
        if(jsonError.error == QJsonParseError::NoError) {
            if(jsonSession.isObject()) {
                QJsonObject rootObj = jsonSession.object();
                QJsonValue tmpModules = rootObj.value("modules");
                if(tmpModules.isArray()) {
                    QJsonArray moduleArray = tmpModules.toArray();
                    for(int i = 0; i < moduleArray.count(); i++) {
                        QJsonValue tmpObject = moduleArray.at(i);
                        if(tmpObject.isObject()) {
                            QJsonObject moduleObject = tmpObject.toObject();
                            QString tmpNameString, tmpConfigString;
                            int moduleId;
                            QFile tmpXmlConfigFile;
                            QByteArray xmlConfigData;

                            tmpNameString = moduleObject.value("name").toString();
                            tmpConfigString = moduleObject.value("configPath").toString();
                            moduleId = moduleObject.value("id").toInt();

                            tmpXmlConfigFile.setFileName(tmpConfigString);
                            if(tmpXmlConfigFile.exists() && tmpXmlConfigFile.open(QIODevice::Unbuffered | QIODevice::ReadOnly)) {
                                xmlConfigData = tmpXmlConfigFile.readAll();
                                tmpXmlConfigFile.close();
                                emit sigLoadModule(tmpNameString, tmpConfigString, xmlConfigData, moduleId);
                            }
                            else {
                                qWarning() << "Error opening config file for module:" << tmpNameString << "path:" << tmpConfigString;
                            }
                        }
                    }
                }
                else {
                    qWarning() << "Error parsing session file:" << t_filePath << "content mismatch";
                }
            }
        }
        else {
            qWarning() << "Error parsing session file:" << t_filePath << "error:" << jsonError.errorString();
        }
    }
    else {
        qWarning() << "Error opening session file:" << t_filePath;
    }
}
