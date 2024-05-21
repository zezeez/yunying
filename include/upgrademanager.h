#ifndef UPGRADEMANAGER_H
#define UPGRADEMANAGER_H
#include <QProgressDialog>

struct FileSizeFmt {
    int main;
    int decimal;
    QString sizeUnit;
};

class UpgradeManager : public QObject
{
    Q_OBJECT
public:
    UpgradeManager(QObject *parent = nullptr);
    ~UpgradeManager();
    QString fileNameFromUrl(const QString &url);
    struct FileSizeFmt fileSizeToHumanNice(qint64 size);
    void setNewVersion(const QString &newVer);
    void setUpgradeUrl(const QString &url);
    void prepareUpgrade();
    void cancelUpgrade();
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void downloadFailed(const QUrl &url);
    void downloadFinished(const QUrl &url, const QByteArray &fileContent);
    int startProcess(const QString &file);
    void doUpgrade(const QString &fileName);
    void upgradeFinished();
    void downloadProgressInit(qint64 receivedBytes, qint64 lastTime);

private:
    QProgressDialog *updatePd;
    QString downloadFileName;
    QString newVersion;
    QString upgradeUrl;
    qint64 lastDownloadTime;
    qint64 lastBytesReceived;
    struct FileSizeFmt lastSizeInSec;
};

#endif // UPGRADEMANAGER_H
