#ifndef USBWORKER_H
#define USBWORKER_H

#include <QThread>

class WorkerThread : public QThread
{
    Q_OBJECT
public:
    WorkerThread(QString name, QString pathSource, QString pathTarget);
    ~WorkerThread();

signals:
    void updateProgress(int nPercent);
    void resultReady(const QString s);

private:
    void run() override;
    bool copyPath(QString sourceDir, QString destinationDir, bool overWriteDirectory);
    bool countFiles(QString sourceDir, QString destinationDir, bool overWriteDirectory, int &nFileCount);

    QString m_strName;
    QString m_pathSource;
    QString m_pathTarget;

    int m_nFileCount = 0;
    int m_nFileCopyCounter = 0;
};



#endif // USBWORKER_H
