#ifndef CMAINWINDOW_H
#define CMAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <ui_cmainwindow.h>
#include <QString>
#include <QThread>

#define PATH_TO_IMAGES QString(QDir::currentPath() + "/images/")
#define USB_KEY_FILE "/keytotheworld"
class QLabel;
class QHBoxLayout;
class CSlideShow;
class CInfoShow;
class QFileSystemModel;
class QFileSystemWatcher;
class WorkerThread;

struct Settings{
    int nTimeImage = 0;
    QString sImagepath = "";
    QString sMotioneyepath = "";
    QString sUsbmount = "";
    bool bAutoStart = false;
    bool bDebug = false;
};

class CMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    CMainWindow(QString version, QWidget *parent = nullptr);
    ~CMainWindow();

private slots:
    void setupUI();
    void autostart();
    void startSlideShow();
    void stopSlideShow();
    void updateSystemInfo();
    void cyclicAction();
    void onStartSlideshow_pressed();

    void copyBtn_pressed();
    void showModifiedDirectory(const QString &);
    void usbInserted(QString pathUsb, bool bStartCopy);
    void checkforUSB();

    void udpateProgressImages(int nPercent);
    void udpateProgressMotionEye(int nPercent);
    void copyImagesfinished();
    void copyMotioneyefinished();
    void copyDone();

    void loadSettings();
    void saveSettings();
    void updateSettingsUI();
    void restartSystem();

private:
    bool isVisionUSB(QString sPathtoUSB);
    bool unMountUSB();
    bool mountUSB();
    bool clearUSB(QString sPathtoClear);
    QString m_sSettingsFile;
    Settings m_Settings;

    Ui::mainWidget m_CentralWidget;
    CSlideShow* m_pSlideShow = nullptr;
    CInfoShow* m_pInfo = nullptr;
    WorkerThread *m_workerImages = nullptr;
    WorkerThread *m_workerMotioneye = nullptr;

    QTimer* m_pUpdateTimer = nullptr;
    QFileSystemModel* m_pImageModel = nullptr;
    QFileSystemModel* m_usbModel = nullptr;
    QFileSystemModel* m_pMotionEyeModel = nullptr;
    QFileSystemWatcher* m_pUSBWatcher = nullptr;

    bool m_bCopyDone = false;
    bool m_bCopyRunning = false;
    bool m_bCopyImagesDone = false;
    bool m_bCopyMotionDone = false;

    QString m_sUSBName = "";
    int m_nSlideShowCounter = -1;
    int m_nSlideShowCounterOld = -1;
};

#endif // CMAINWINDOW_H

