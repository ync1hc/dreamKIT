#ifndef AIASSIT_H
#define AIASSIT_H

#include "QString"
#include <QObject>
#include <QThread>
#include <QList>
#include <QFileSystemWatcher>
#include <QTimer>

class AiassistAsync;

class VssThread : public QThread
{
    Q_OBJECT

public:
    VssThread(AiassistAsync *parent);
    void run();

private:
    AiassistAsync *m_parent;

    QString getString_VssApiValue(QString apiName, QString &currentTimeStamp);
    bool getBool_VssApiValue(QString apiName, QString &currentTimeStamp);
    u_int32_t getUint32_VssApiValue(QString apiName, QString &currentTimeStamp);

};

class AiassistAsync: public QObject
{
    Q_OBJECT
public:
    AiassistAsync();
    ~AiassistAsync();

    Q_INVOKABLE void setTextToSpeech(QString msg);

signals:
    void updateTextToSpeech(QString msg);
    void setSecurityIsAttacked(bool sts);
    void setSecurityReactionStage(int secReact);


private:
    VssThread *vssThread;
};

#endif // AIASSIT_H
