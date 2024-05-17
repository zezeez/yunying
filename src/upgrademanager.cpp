#include "upgrademanager.h"
#include "nethelper.h"
#include <QFile>
#include <QMessageBox>
#include <QUrl>
#include <QProcess>

UpgradeManager::UpgradeManager(QObject *parent) : QObject(parent)
{
    updatePd = nullptr;
}

UpgradeManager::~UpgradeManager()
{
    upgradeFinished();
}

QString UpgradeManager::fileNameFromUrl(const QString &url)
{
    QString fileName;

    if (!url.isEmpty()) {
        int idx = url.lastIndexOf('/');
        if (idx != -1 && idx != url.size() - 1) {
             fileName = url.sliced(idx + 1);
        }
        if (fileName.isEmpty()) {
             fileName = _("yuny.zip");
        }
    } else {
        fileName = _("yuny.zip");
    }
    return fileName;
}

struct FileSizeFmt UpgradeManager::fileSizeToHumanNice(qint64 size)
{
    QStringList sizeUnit = {
        "B",
        "KB",
        "MB",
        "GB",
        "TB",
        "EB"
    };

    int sizeIdx = 0;
    qint64 calcuSize = size;
    while ((calcuSize >> 10) > 0) {
        calcuSize >>= 10;
        sizeIdx++;
    }
    struct FileSizeFmt fsf;
    fsf.main = calcuSize;
    fsf.decimal = ((size & 1023) * 10) >> 10;
    fsf.sizeUnit = sizeIdx < sizeUnit.size() ? sizeUnit[sizeIdx] : sizeUnit[sizeUnit.size() - 1];
    return fsf;
}

void UpgradeManager::setNewVersion(const QString &newVer)
{
    newVersion = newVer;
}

void UpgradeManager::setUpgradeUrl(const QString &url)
{
    upgradeUrl = url;
}

void UpgradeManager::prepareUpgrade()
{
    if (upgradeUrl.isEmpty() || newVersion.isEmpty())
        return;

    if (!updatePd) {
        updatePd = new QProgressDialog(tr("正在更新..."), tr("取消"), 0, 105);
        connect(updatePd, &QProgressDialog::canceled, this, &UpgradeManager::cancelUpgrade);
        downloadFileName = fileNameFromUrl(upgradeUrl);
        updatePd->setLabelText(_("正在下载更新文件%1").arg(downloadFileName));
        NetHelper::instance()->downloadFile(upgradeUrl);
    }
}

void UpgradeManager::cancelUpgrade()
{
    NetHelper::instance()->cancelDownload();
}

void UpgradeManager::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (bytesTotal > 0) {
        int v = bytesReceived * 100 / bytesTotal;
        updatePd->setValue(v);
        struct FileSizeFmt r = fileSizeToHumanNice(bytesReceived);
        struct FileSizeFmt t = fileSizeToHumanNice(bytesTotal);
        updatePd->setLabelText(_("正在下载更新文件%1        已下载: %2.%3 %4/总共: %5.%6 %7")
                                   .arg(downloadFileName)
                                   .arg(r.main)
                                   .arg(r.decimal)
                                   .arg(r.sizeUnit)
                                   .arg(t.main)
                                   .arg(t.decimal)
                                   .arg(t.sizeUnit));
    } else {
        updatePd->setValue(0);
        struct FileSizeFmt r = fileSizeToHumanNice(bytesReceived);
        updatePd->setLabelText(_("正在下载更新文件%1        已下载: %2.%3 %4/总共: 未知")
                                   .arg(downloadFileName)
                                   .arg(r.main)
                                   .arg(r.decimal)
                                   .arg(r.sizeUnit));
    }
}

void UpgradeManager::downloadFailed(const QUrl &url)
{
    upgradeFinished();
    QMessageBox::warning(nullptr, tr("更新失败"), tr("更新文件下载失败，请前往 <a href=%1>%2</a> 手动下载更新文件.").arg(url.url(), url.url()));
}

void UpgradeManager::downloadFinished(const QUrl &url, const QByteArray &fileContent)
{
    QFile file;
    QString fileName;

    fileName = _("../") + fileNameFromUrl(url.url());
    file.setFileName(fileName);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(fileContent);
        file.close();
        doUpgrade(fileName);
    } else {
        QMessageBox::warning(nullptr, tr("更新失败"), tr("%1: 无法打开文件写入").arg(fileName));
    }
}

int UpgradeManager::startProcess(const QString &file)
{
    QProcess *p = new QProcess;
    connect(p, &QProcess::started, this, []() {
        exit(0);
    });
    connect(p, &QProcess::errorOccurred, this, [p](QProcess::ProcessError error) {
        QMessageBox::warning(nullptr, tr("更新失败"), tr("无法启动目标进程: %1").arg(error));
        p->deleteLater();
    });
    p->start(file);
    //p->waitForStarted();

    return 0;
}

void UpgradeManager::doUpgrade(const QString &fileName)
{
    updatePd->setLabelText(_("正在解压文件%1").arg(fileName));
#ifdef Q_OS_WIN
    if (!QFile::exists(_("./7za.exe"))) {
        QMessageBox::warning(nullptr, tr("更新失败"), tr("缺少./7za.exe文件"));
#else
    if (!QFile::exists(_("./7za"))) {
        QMessageBox::warning(nullptr, tr("更新失败"), tr("缺少./7za文件"));
#endif
        return;
    }

    QString command = _("./7za");
    int ret = QProcess::execute(command, {_("x"), _("-y"), _("-o.."), fileName});
    if (ret != 0) {
        upgradeFinished();
        QMessageBox::warning(nullptr, tr("更新失败"), tr("%1: 解压文件失败").arg(fileName));
        return;
    }

    upgradeFinished();

#ifdef Q_OS_WIN
    command = _("../yuny_%1/yuny.exe").arg(newVersion);
#else
    command = _("../yuny_%1/yuny").arg(newVersion);
#endif
    if (!QFile::exists(command)) {
        QMessageBox::warning(nullptr, tr("更新失败"), tr("%1: 文件不存在").arg(command));
        return;
    }

    ret = startProcess(command);
    if (ret == 0) {
        //exit(0);
    } else {
        QMessageBox::warning(nullptr, tr("更新失败"), tr("无法启动更新后的进程"));
    }
}

void UpgradeManager::upgradeFinished()
{
    if (updatePd) {
        delete updatePd;
        updatePd = nullptr;
    }
}
