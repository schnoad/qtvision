#include "cslideshow.h"
#include <QLayout>
#include <QLabel>
#include <QGuiApplication>
#include <QFile>
#include <QDir>
#include <QScreen>
#include <QDebug>
#include <QString>
#include <QHostInfo>

CSlideShow::CSlideShow(int i_nSec, QString sPath, QWidget *parent)
    : QMainWindow(parent)
    , m_sImagePath(sPath)
    , m_nImageCount(0)
    , m_nTimerId (0)
{
//    this->setAttribute(Qt::WA_DeleteOnClose);
    if(QHostInfo::localHostName().contains("pi"))
    {
        setCursor(Qt::BlankCursor);
        //Needed window flags for only show this widget without frame on top
        setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
        setWindowState(Qt::WindowFullScreen);
    }

    setupUI();

    recScreen = QGuiApplication::primaryScreen()->geometry();

    // create a timer
    m_pTimer = new QTimer(this);

    // setup signal and slot
    connect(m_pTimer, SIGNAL(timeout()),
          this, SLOT(timerSlot()));

    //at least 9 sec for loading new image
    if(i_nSec < 9)
        i_nSec = 9;

    // msec
    m_pTimer->start(i_nSec * 1000);
    m_nTimerId = m_pTimer->timerId();
}

void CSlideShow::setupUI()
{
    m_pLabel = new QLabel(this);
    m_pLabel->setText("Starting Slideshow ...");

    this->setCentralWidget(m_pLabel);
}

CSlideShow::~CSlideShow()
{
    qDebug() << "CImage::~CImage()";
    disconnect(m_pTimer, SIGNAL(timeout()),
          this, SLOT(timerSlot()));

    m_pTimer->stop();
    delete m_pTimer;
}

void CSlideShow::show()
{
    QWidget::show();
}

void CSlideShow::hide()
{
    QWidget::hide();
}

void CSlideShow::changeBackground(QString i_sPath)
{
    if(!QFile::exists(i_sPath))
    {
        qDebug() << "File does not exist " + i_sPath;
        return;
    }
    qDebug() << i_sPath;

    QImage myImage;
    if(myImage.load(i_sPath))
    {
        myImage = myImage.scaled(recScreen.width(), recScreen.height(), Qt::KeepAspectRatio);

        int x = 960;
        x = (recScreen.width ()) / 2;

        int y = 512;
        y = (recScreen.height ()) / 2;

        QString str = "x/y=" + QString::number(x) + "/" +  QString::number(y);
        qDebug() << str;

        m_pLabel->setGeometry(recScreen);

        m_pLabel->setPixmap(QPixmap::fromImage(myImage));
        m_pLabel->show();
    }
    else
    {
        m_pLabel->setText("failed to load image: " + i_sPath);
        m_pLabel->show();
    }

    show();
}


void CSlideShow::timerSlot()
{
    QDir directory = m_sImagePath;
    QStringList images = directory.entryList(QStringList() << "*.jpg" << "*.JPG", QDir::Files);

    qDebug() << "check path: " + directory.path();

    if(images.size())
    {
        if(m_nImageCount >= images.size())
            m_nImageCount = 0;

        changeBackground(directory.path() + "/" + images[m_nImageCount]);

        m_nImageCount++;
        m_nCounter++;
    }
    else
        qDebug() << "no files found in path: " + directory.path();
}

//**************

CInfoShow::CInfoShow(QWidget *parent)
    : QMainWindow(parent)
{
    if(QHostInfo::localHostName().contains("pi"))
    {
        setCursor(Qt::BlankCursor);

        //Needed window flags for only show this widget without frame on top
        setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
        setWindowState(Qt::WindowFullScreen);
    }

    setupUI();
}

void CInfoShow::setupUI()
{
    m_pLabel = new QLabel(this);

    recScreen = QGuiApplication::primaryScreen()->geometry();

    QImage myImage;
    if(myImage.load(":/data/remove"))
    {
        myImage = myImage.scaled(recScreen.width(), recScreen.height(), Qt::KeepAspectRatio);

        m_pLabel->setGeometry(recScreen);
        m_pLabel->setPixmap(QPixmap::fromImage(myImage));
        m_pLabel->show();
    }
    else
    {
        m_pLabel->setText("Please remove USB Stick, Sytem is going to restart in 30 sec ...");
    }

    this->setCentralWidget(m_pLabel);
}

CInfoShow::~CInfoShow()
{
}

void CInfoShow::show()
{
    qDebug() << "CInfoShow::show()";
    QWidget::show();
}

void CInfoShow::hide()
{
    qDebug() << "CInfoShow::hide()";
    QWidget::hide();
}
