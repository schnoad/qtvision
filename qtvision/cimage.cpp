#include "cimage.h"
#include <QLayout>
#include <QLabel>
#include <QGuiApplication>
#include <QFile>
#include <QDir>
#include <QScreen>
#include <QDebug>
#include <QString>

CImage::CImage(QWidget *parent)
    : QMainWindow(parent)
    , m_nImageCount(0)
{
    setupUI();

    // create a timer
    m_pTimer = new QTimer(this);

    // setup signal and slot
    connect(m_pTimer, SIGNAL(timeout()),
          this, SLOT(timerSlot()));

    // msec
    m_pTimer->start(1000);
}

CImage::~CImage()
{
}

void CImage::changeBackground(QString i_sPath)
{
    if(!QFile::exists(i_sPath))
    {
        qDebug() << "File does not exist " + i_sPath;
        return;
    }
    qDebug() << i_sPath;

    QImage myImage;
    myImage.load(i_sPath);

    QRect recScreen = QGuiApplication::primaryScreen()->geometry();
    myImage = myImage.scaled(recScreen.width(), recScreen.height(), Qt::KeepAspectRatio);

    int x = 960;
//    if(recScreen.width() != myImage.width())
//        x = (recScreen.width () - myImage.width ()) / 2;

    int y = 512;
//    if(recScreen.height() != myImage.height())
//        y = (recScreen.height () - myImage.height ()) / 2;

    QString str = "x/y=" + QString::number(x) + "/" +  QString::number(y);
    qDebug() << str;

    //m_pLabel->setGeometry(x,y,recScreen.width(),recScreen.height());
    m_pLabel->setGeometry(recScreen);

    m_pLabel->setPixmap(QPixmap::fromImage(myImage));
    m_pLabel->show();
}

void CImage::setupUI()
{
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    setWindowState(Qt::WindowFullScreen);

    m_pLabel = new QLabel();
    m_pLabel->setText("Starting ...");

    this->setCentralWidget(m_pLabel);
}

void CImage::timerSlot()
{
    QDir directory = QDir::currentPath() + "/images/";//("/home/andsch/vision/prog/vision/images/");
    QStringList images = directory.entryList(QStringList() << "*.jpg" << "*.JPG", QDir::Files);

    qDebug() << "check path: " + directory.path();

    if(images.size())
    {
        if(m_nImageCount >= images.size())
            m_nImageCount = 0;

        changeBackground(directory.path() + "/" + images[m_nImageCount]);

        m_nImageCount++;
    }
    else
        qDebug() << "no files found in path: " + directory.path();
}
