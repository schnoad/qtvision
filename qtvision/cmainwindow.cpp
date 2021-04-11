#include "cmainwindow.h"
#include "cslideshow.h"
#include "usbworker.h"
#include "server.h"

#include <QLayout>
#include <QLabel>
#include <QGuiApplication>
#include <QFile>
#include <QDir>
#include <QScreen>
#include <QDebug>
#include <QPushButton>
#include <QStorageInfo>
#include <QFileSystemModel>
#include <QFileSystemWatcher>
#include <QProcess>
#include <QSettings>
#include <QHostInfo>

/*HELP
 *
 * working dir on raspberry: /home/pi/vision
 *
 * sudo chmod 700
 * sudo chown pi:pi
 *
 * autostart
sudo nano .config/lxsession/LXDE-pi/autostart

    @lxpanel --profile LXDE-pi
    @pacmanfm --desktop --profile LXDE-pi

    # Run a sample bash script
    #@sh /home/pi/your/script.sh

    @xset s noblank
    @xset s off
    @xset x -dpms

    # Or a sample python script
    #@sudo python3 /home/pi/vision/main_vision.py & disown
    sudo /home/pi/vision/vision & disown
*/



CMainWindow::CMainWindow(QString version, QWidget *parent)
    : QMainWindow(parent)
    , m_sSettingsFile(QDir::currentPath() + "/settings.ini")
{
    qDebug() << "CMainWindow::CMainWindow()";
    QString hostName = QHostInfo::localHostName();
    qDebug() << "Running on " << hostName;

    if(hostName.contains("pi"))
    {
        m_sSettingsFile = "/home/pi/vision/settings.ini";
    }

    setWindowTitle("vision " + version);
//    this->setAttribute(Qt::WA_DeleteOnClose);
    setCursor(Qt::BlankCursor);

    loadSettings();

    setupUI();

    updateSettingsUI();

    setWindowFlags(Qt::FramelessWindowHint);
    setWindowState(Qt::WindowFullScreen);

    // Autostart
    if(m_Settings.bAutoStart)
    {
        qDebug() << "CMainWindow::CMainWindow() bAutoStart";
        QTimer::singleShot(10000, this, &CMainWindow::autostart);
    }

    // create a timer
    m_pUpdateTimer = new QTimer(this);

    // setup signal and slot
    connect(m_pUpdateTimer, SIGNAL(timeout()),
          this, SLOT(cyclicAction()));

    // msec
    m_pUpdateTimer->start(11000);

    // prepare FS watcher to detect added USB memory
    // is not working on current raspian
//    if(!m_Settings.sUsbmount.isEmpty())
//    {
//        m_pUSBWatcher = new QFileSystemWatcher(QStringList() << m_Settings.sUsbmount, this);
//        connect(m_pUSBWatcher, SIGNAL(directoryChanged(const QString&)),
//                this, SLOT(showModifiedDirectory(const QString&)));
//    }

    connect(m_CentralWidget.pushButton_StartSlideshow, SIGNAL(pressed()),
            this, SLOT(onStartSlideshow_pressed()));

    connect(m_CentralWidget.copyUSBBtn, SIGNAL(pressed()),
            this, SLOT(copyBtn_pressed()));

    connect(m_CentralWidget.pushButton_LoadSettings, SIGNAL(pressed()),
            this, SLOT(loadSettings()));

    connect(m_CentralWidget.pushButton_SaveSettings, SIGNAL(pressed()),
            this, SLOT(saveSettings()));

    connect(m_CentralWidget.pushButton_Restart, SIGNAL(pressed()),
            this, SLOT(restartSystem()));

    cyclicAction();
}

void CMainWindow::setupUI()
{
    qDebug() << "CMainWindow::setupUI()";
    QWidget* mainWidget = new QWidget();
    m_CentralWidget.setupUi(mainWidget);

    m_CentralWidget.groupBoxUSB->setVisible(false);

    if(m_Settings.bDebug)
    {
        m_CentralWidget.groupBoxSystemInfo->setVisible(true);
        m_CentralWidget.copyUSBBtn->setVisible(true);
    }
    else
    {
        m_CentralWidget.groupBoxSystemInfo->setVisible(false);
        m_CentralWidget.copyUSBBtn->setVisible(false);
        m_CentralWidget.groupBoxSettings->setEnabled(false);
        m_CentralWidget.pushButton_LoadSettings->setVisible(false);
        m_CentralWidget.pushButton_SaveSettings->setVisible(false);
        m_CentralWidget.pushButton_Restart->setVisible(false);
    }

    this->setCentralWidget(mainWidget);
}

CMainWindow::~CMainWindow()
{
    qDebug() << "CMainWindow::~CMainWindow()";

    if(m_pSlideShow)
    {
        m_pSlideShow->close();
        delete m_pSlideShow;
    }
    if(m_pInfo)
    {
        m_pInfo->close();
        delete m_pInfo;
    }
}

void CMainWindow::autostart()
{
    qDebug() << "CMainWindow::autostart()";
    startSlideShow();
}

void CMainWindow::startSlideShow()
{
    qDebug() << "CMainWindow::startSlideShow()";
    if(!m_pSlideShow)
    {
        if(m_pInfo)
        {
            m_pInfo->close();
            delete m_pInfo;
            m_pInfo = nullptr;
        }

        qDebug() << "create CImage";
        m_CentralWidget.pushButton_StartSlideshow->setText("Stop Slideshow");
        m_pSlideShow = new CSlideShow(m_Settings.nTimeImage, m_Settings.sImagepath);
    }

    m_pSlideShow->show();
    this->hide();
}

void CMainWindow::stopSlideShow()
{
    qDebug() << "CMainWindow::stopSlideShow()";
    if(m_pSlideShow != nullptr)
    {
        m_pSlideShow->close();
        delete m_pSlideShow;
        m_pSlideShow = nullptr;
        m_CentralWidget.pushButton_StartSlideshow->setText("Start Slideshow");
    }

    this->show();
}

void CMainWindow::updateSystemInfo()
{
    if(m_pSlideShow)
    {
       return;
    }

    qDebug() << "CMainWindow::updateSystemInfo()" << "Path to Images: " << m_Settings.sImagepath << "Path to MotionEye: " << m_Settings.sMotioneyepath;

//    if(!m_Settings.sMotioneyepath.isEmpty())
//    {
//        m_pMotionEyeModel = new QFileSystemModel(this);
//        m_pMotionEyeModel->setReadOnly(true);
//        m_pMotionEyeModel->setFilter(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);
//        m_pMotionEyeModel->setRootPath(m_Settings.sImagepath);

//        m_CentralWidget.motioneyeView->setModel(m_pMotionEyeModel);
//        m_CentralWidget.motioneyeView->setRootIndex(m_pMotionEyeModel->index(m_Settings.sMotioneyepath));
//    }

    if(!m_Settings.sImagepath.isEmpty())
    {
        m_pImageModel = new QFileSystemModel(this);
        m_pImageModel->setReadOnly(true);
        m_pImageModel->setFilter(QDir::NoDotAndDotDot | QDir::Files);
        m_pImageModel->setRootPath(m_Settings.sImagepath);

        m_CentralWidget.imageView->setModel(m_pImageModel);
        m_CentralWidget.imageView->setRootIndex(m_pImageModel->index(m_Settings.sImagepath));
    }

    if(m_Settings.bDebug)
    {
        //Some System Information
        QStorageInfo info(m_Settings.sImagepath);

        QString strAvailableSpaceMB;
        strAvailableSpaceMB = strAvailableSpaceMB.setNum(int(info.bytesAvailable()/1000/1000)) + "[MB]";
        m_CentralWidget.availablespaceMB->setText(strAvailableSpaceMB);

        QString strUsedSpaceMB;
        strUsedSpaceMB = strUsedSpaceMB.setNum(int( (info.bytesTotal()- info.bytesAvailable())/1000/1000)) + "[MB]";
        m_CentralWidget.usedspaceMB->setText(strUsedSpaceMB);

        QString strtotalSpaceMB;
        strtotalSpaceMB = strtotalSpaceMB.setNum(int( info.bytesTotal()/1000/1000)) + "[MB]";
        m_CentralWidget.totalspaceMB->setText(strtotalSpaceMB);

        m_CentralWidget.availablespace->setValue(int( info.bytesTotal()/1000/1000) - int( (info.bytesAvailable())/1000/1000));
    }
}

void CMainWindow::cyclicAction()
{
    updateSystemInfo();
    checkforUSB();
}

void CMainWindow::showModifiedDirectory(const QString& /*path*/)
{
}

void CMainWindow::usbInserted(QString pathUsb, bool bStartCopy)
{
    qDebug() << "CMainWindow::usbInserted() " << pathUsb << " bStartCopy: " << bStartCopy << " workerImages" << (m_workerImages != nullptr) << " workerMotion" << (m_workerMotioneye != nullptr);

    stopSlideShow();
    //copy images from usb
    {
        qDebug() << "CMainWindow::usbInserted() update USB UI";
        QString sPathToImagesOnUsb = pathUsb;

//        m_usbModel = new QFileSystemModel(this);
//        m_usbModel->setReadOnly(true);
//        m_usbModel->setFilter(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);
//        m_usbModel->setRootPath(sPathToImagesOnUsb);

//        m_CentralWidget.usbView->setModel(m_usbModel);
//        m_CentralWidget.usbView->setRootIndex(m_usbModel->index(sPathToImagesOnUsb + "/data/"));

        m_CentralWidget.groupBoxUSB->setVisible(true);

        if(bStartCopy &&
                !m_bCopyRunning &&
                !m_bCopyDone &&
                m_workerImages == nullptr &&
                m_workerMotioneye == nullptr)
        {
            qDebug() << "CMainWindow::usbInserted() StartCopy";
            m_bCopyRunning = true;
            m_bCopyImagesDone = false;
            m_bCopyMotionDone = false;
            m_sUSBName = pathUsb;

            m_CentralWidget.groupBoxUSB->setVisible(true);

            {
                qDebug() << "CMainWindow::usbInserted() Files to Transfer from " << sPathToImagesOnUsb + "/data/" << " to: " << m_Settings.sImagepath;

                //need to clear the destination/target
                clearUSB(m_Settings.sImagepath);

                m_workerImages = new WorkerThread("images", sPathToImagesOnUsb + "/data/", m_Settings.sImagepath);
                connect(m_workerImages, &WorkerThread::finished, this, &CMainWindow::copyImagesfinished);
                connect(m_workerImages, SIGNAL(updateProgress(int)), this, SLOT(udpateProgressImages(int)));
            }

            m_workerImages->start();

            //copy motioneye surveillance to usb
            {
                QString sPathToMotioneyeOnUsb = pathUsb + "/cam/";

                qDebug() << "CMainWindow::usbInserted() Files to Transfer from " << m_Settings.sMotioneyepath << " to: " << sPathToMotioneyeOnUsb;

                //need to clear the destination/target
                QString sPathToMotioneyeOnUsbClear = pathUsb + "/cam/*";
                clearUSB(sPathToMotioneyeOnUsbClear);

                m_workerMotioneye = new WorkerThread("motion", m_Settings.sMotioneyepath, sPathToMotioneyeOnUsb);
                connect(m_workerMotioneye, &WorkerThread::finished, this, &CMainWindow::copyMotioneyefinished);
                connect(m_workerMotioneye, SIGNAL(updateProgress(int)), this, SLOT(udpateProgressMotionEye(int)));

                m_workerMotioneye->start();
            }
        }
    }
}

void CMainWindow::checkforUSB()
{
    qDebug() << "CMainWindow::checkforUSB() path: " << m_Settings.sUsbmount;

    if(m_Settings.sUsbmount.isEmpty() && !m_bCopyRunning)
        return;

    if(mountUSB())
    {
        QDir directory = m_Settings.sUsbmount;
        QStringList images = directory.entryList(QStringList(), QDir::NoDotAndDotDot | QDir::Dirs);

        if(images.isEmpty())
             qDebug() << "CMainWindow::checkforUSB() no USB found";

        foreach(QString dir, images)
        {
            QString sUSBName = directory.path() + "/" + dir;
            qDebug() << "CMainWindow::checkforUSB() found usb: " + sUSBName;

            if(!isVisionUSB(sUSBName))
            {
                qDebug() << "CMainWindow::usbInserted() no visionUSB";
                return;
            }

            usbInserted(sUSBName, !m_Settings.bDebug);
            return;
        }
    }

    qDebug() << "CMainWindow::checkforUSB() no Usb images Inserted!";
    m_CentralWidget.groupBoxUSB->setVisible(false);
    m_bCopyDone = false;
}

void CMainWindow::udpateProgressImages(int nPercent)
{
    qDebug() << "CMainWindow::udpateProgressImages Percent: " << nPercent;
    m_CentralWidget.progressBarUSB_Images->setValue(nPercent);
}

void CMainWindow::udpateProgressMotionEye(int nPercent)
{
    qDebug() << "CMainWindow::udpateProgressMotionEye Percent: " << nPercent;
    m_CentralWidget.progressBarUSB_MotionEye->setValue(nPercent);
}

//Code snippets
//QProcess* process = new QPr/*ocess();

//process->start("mkdir /media/pi/myusb");
//process->waitForFinished();
//process->start("sudo mount /dev/sda /media/pi/myusb");
//process->waitForFinished();*/

void CMainWindow::onStartSlideshow_pressed()
{
     qDebug() << "CMainWindow::onStartSlideshow_pressed";

     if(m_pSlideShow == nullptr)
     {
         startSlideShow();
     }
     else
     {
         stopSlideShow();
     }
}

void CMainWindow::copyBtn_pressed()
{
    checkforUSB();
}

void CMainWindow::copyImagesfinished()
{
    qDebug() << "CMainWindow::copyImagesfinished() ";
    m_workerImages->deleteLater();
    m_workerImages = nullptr;
    m_bCopyImagesDone = true;
    copyDone();
}

void CMainWindow::copyMotioneyefinished()
{
    qDebug() << "CMainWindow::copyMotioneyefinished() ";
    m_workerMotioneye->deleteLater();
    m_workerMotioneye = nullptr;
    m_bCopyMotionDone = true;
    copyDone();
}

void CMainWindow::copyDone()
{
    qDebug() << "CMainWindow::copyDone ...";
    if( m_bCopyImagesDone && m_bCopyMotionDone)
    {
        qDebug() << "CMainWindow::copyDone all Done";
        m_bCopyRunning = false;
        m_bCopyDone = true; //no more copy when its already done once
        m_CentralWidget.groupBoxUSB->setVisible(false);

        //unmount usb stick
        unMountUSB();

        //update the ui models
        updateSystemInfo();

        if(!m_pInfo)
        {
            m_pInfo = new CInfoShow();
        }

        m_pInfo->show();

        if(m_Settings.bAutoStart)
        {
            qDebug() << "CMainWindow::copyDone restartSystem after copy";
            QTimer::singleShot(30000, this, &CMainWindow::restartSystem);
        }
        else
        {
            qDebug() << "CMainWindow::copyDone no autostart after copy";
        }
    }
}

void CMainWindow::loadSettings()
{
    qDebug() << "CMainWindow::loadSettings" << m_sSettingsFile;
    QSettings settings(m_sSettingsFile, QSettings::IniFormat);
    QString version = settings.value("version", "NA").toString();
    m_Settings.sImagepath = settings.value("imagepath", PATH_TO_IMAGES).toString();
    m_Settings.sUsbmount = settings.value("usbmount", "/media/pi/").toString();
    m_Settings.nTimeImage = settings.value("timeimage", int(5)).toInt();
    m_Settings.bAutoStart = settings.value("autostart", false).toBool();
    m_Settings.bDebug = settings.value("bDebug", false).toBool();
    m_Settings.sMotioneyepath = settings.value("motioneye", "/mnt/cam/").toString();

    qDebug() << version << m_Settings.sImagepath << m_Settings.sUsbmount << m_Settings.nTimeImage << m_Settings.bDebug << m_Settings.bAutoStart;
}

void CMainWindow::saveSettings()
{
    qDebug() << "CMainWindow::saveSettings fileName: " << m_sSettingsFile;
    QSettings settings(m_sSettingsFile, QSettings::IniFormat);
    settings.setValue("version", "b0001");
    settings.setValue("imagepath", m_CentralWidget.editImagePath->text());
    settings.setValue("usbmount", m_CentralWidget.editUsbMount->text());
    settings.setValue("motioneye", m_CentralWidget.editMotioneye->text());
    settings.setValue("timeimage", m_CentralWidget.spinBoxTimeImage->value());
    settings.setValue("autostart", m_CentralWidget.checkBoxAutostart->isChecked());
    settings.setValue("bDebug", m_Settings.bDebug);
}

void CMainWindow::updateSettingsUI()
{
    qDebug() << "CMainWindow::updateSettingsUI ";
    m_CentralWidget.editMotioneye->setText(m_Settings.sMotioneyepath);
    m_CentralWidget.editImagePath->setText(m_Settings.sImagepath);
    m_CentralWidget.imagePath->setText("Path to Images: " + m_Settings.sImagepath);
    m_CentralWidget.editUsbMount->setText(m_Settings.sUsbmount);
    m_CentralWidget.spinBoxTimeImage->setValue(m_Settings.nTimeImage);

    m_CentralWidget.checkBoxAutostart->setChecked(m_Settings.bAutoStart);
    if(m_Settings.bAutoStart)
        m_CentralWidget.checkBoxAutostart->setText("On");
    else
        m_CentralWidget.checkBoxAutostart->setText("Off");

}

void CMainWindow::restartSystem()
{
    //check if we are running on pi -> if yes restart system "sudo restart now"
    QString hostName = QHostInfo::localHostName();
    qDebug() << "Running on " << hostName;

    stopSlideShow();

    if(hostName.contains("pi"))
    {
        //check if usb stick with key is mounted
        //-> yes close application
        //-> no restart system
        qDebug() << "going to restart the system";
        system("reboot now");
    }

    qDebug() << "going to close application";
    QWidget::close();
    QWidget::destroy();
}

bool CMainWindow::isVisionUSB(QString sPathtoUSB)
{
    qDebug() << "isVisionUSB file exist:" << QFile::exists(sPathtoUSB) << " key exist:" << (sPathtoUSB + USB_KEY_FILE) << QFile::exists(sPathtoUSB + USB_KEY_FILE);
    if(QFile::exists(sPathtoUSB))
    {
        if(QFile::exists(sPathtoUSB + USB_KEY_FILE))
        {
            QFileInfo my_dir(sPathtoUSB);

            if(my_dir.isDir() && my_dir.isWritable())
            {
                // Do something
                //check if keytotheworld exists
                qDebug() << "isVisionUSB Key found";
                return true;
            }
        }
    }

    qDebug() << "isVisionUSB no key found";
    return false;
}

bool CMainWindow::unMountUSB()
{
    qDebug() << "unMountUSB" << m_sUSBName;
    QStringList parameters;
    parameters << m_sUSBName;
    QProcess ps;
    ps.start("umount", parameters);

    return ps.waitForFinished(5000);
}

bool CMainWindow::mountUSB()
{
    qDebug() << "mountUSB" << "/dev/sda1" << "/media/pi/usbmount";
    QStringList parameters;
    parameters << "/dev/sda1" << "/media/pi/usbmount";
    QProcess ps;
    ps.start("mount", parameters);

    return ps.waitForFinished(5000);
}

bool CMainWindow::clearUSB(QString sPathtoClear)
{
    qDebug() << "rm -rf" << sPathtoClear;
    QStringList parameters;
    parameters << sPathtoClear;
    QProcess ps;
    ps.start("rm -rf", parameters);

    return ps.waitForFinished(10000);
}
