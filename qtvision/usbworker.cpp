#include "usbworker.h"

#include <QDir>
#include <QFile>
#include <QDebug>



WorkerThread::WorkerThread(QString name, QString pathSource, QString pathTarget) : QThread()
  , m_strName(name)
  , m_pathSource (pathSource)
  , m_pathTarget (pathTarget)
{

}

WorkerThread::~WorkerThread()
{
    qDebug() << "\n ~WorkerThread() \n";
}

bool WorkerThread::countFiles(QString sourceDir, QString destinationDir, bool overWriteDirectory, int &nFileCount)
{
    QDir originDirectory(sourceDir);

    if (! originDirectory.exists())
    {
        qDebug() << "WorkerThread::countFiles failed originDirectory" << m_strName;
        return false;
    }

    QDir destinationDirectory(destinationDir);

    if(destinationDirectory.exists() && !overWriteDirectory)
    {
        qDebug() << "WorkerThread::countFiles failed originDirectory" << m_strName;
        return false;
    }
    else if(destinationDirectory.exists() && overWriteDirectory)
    {
        destinationDirectory.removeRecursively();
    }

    originDirectory.mkpath(destinationDir);

    foreach (QString directoryName, originDirectory.entryList(QDir::Dirs | \
                                                              QDir::NoDotAndDotDot))
    {
        QString destinationPath = destinationDir + "/" + directoryName;
        originDirectory.mkpath(destinationPath);
        countFiles(sourceDir + "/" + directoryName, destinationPath, overWriteDirectory, nFileCount);
    }

    foreach (QString fileName, originDirectory.entryList(QDir::Files))
    {
        nFileCount++;
        qDebug() << "FileCount: " << nFileCount << " fileName: " << fileName;
    }

    /*! Possible race-condition mitigation? */
    QDir finalDestination(destinationDir);
    finalDestination.refresh();

    if(finalDestination.exists())
    {
        return true;
    }

    return false;
}



bool WorkerThread::copyPath(QString sourceDir, QString destinationDir, bool overWriteDirectory)
{
    QDir originDirectory(sourceDir);

    if (! originDirectory.exists())
    {
        return false;
    }

    QDir destinationDirectory(destinationDir);

    if(destinationDirectory.exists() && !overWriteDirectory)
    {
        return false;
    }
    else if(destinationDirectory.exists() && overWriteDirectory)
    {
        destinationDirectory.removeRecursively();
    }

    originDirectory.mkpath(destinationDir);

    foreach (QString directoryName, originDirectory.entryList(QDir::Dirs | \
                                                              QDir::NoDotAndDotDot))
    {
        QString destinationPath = destinationDir + "/" + directoryName;
        originDirectory.mkpath(destinationPath);
        copyPath(sourceDir + "/" + directoryName, destinationPath, overWriteDirectory);
    }

    foreach (QString fileName, originDirectory.entryList(QDir::Files))
    {
        qDebug() << "WorkerThread" << m_strName << " source: " << (sourceDir + "/" + fileName) << "dest: " << (destinationDir + "/" + fileName);
        QFile::copy(sourceDir + "/" + fileName, destinationDir + "/" + fileName);
        m_nFileCopyCounter++;
        double dPercent = double(m_nFileCopyCounter) / double(m_nFileCount);
        qDebug() << "WorkerThread" << m_strName << " done: " << dPercent;
        emit updateProgress(dPercent * 100.0);
    }

    /*! Possible race-condition mitigation? */
    QDir finalDestination(destinationDir);
    finalDestination.refresh();

    if(finalDestination.exists())
    {
        return true;
    }

    return false;
}

void WorkerThread::run()
{
    QString result = "Ok";

    qDebug() << "WorkerThread::run Copy ..." << m_strName;

    emit updateProgress(0);

    //need to get a progress
    m_nFileCount = 0;
    m_nFileCopyCounter = 0;
    if(!countFiles(m_pathSource, m_pathTarget, true, m_nFileCount))
        qDebug() << "WorkerThread::countFiles failed" << m_strName;

    qDebug() << "WorkerThread::run FileCount: " << m_nFileCount << m_strName;

    copyPath(m_pathSource, m_pathTarget, true);

    emit updateProgress(100);

    qDebug() << "WorkerThread::run emit resultReady" << m_strName;
    emit resultReady(result);
}
