#ifndef CIMAGE_H
#define CIMAGE_H

#include <QMainWindow>
#include <QTimer>

class QLabel;
class QHBoxLayout;



class CSlideShow : public QMainWindow
{
    Q_OBJECT
public:
    CSlideShow(int i_nSec = 30, QString sPath = "", QWidget *parent = nullptr);
    ~CSlideShow();

    void show();
    void hide();

    int getCounter() { return m_nCounter; }

private slots:
    void setupUI();
    void timerSlot();

private:
    void changeBackground(QString i_sPath);

    QString m_sImagePath;

    QTimer* m_pTimer;
    QLabel* m_pLabel;

    int m_nImageCount = 0;
    int m_nTimerId = 0;
    int m_nCounter = 1;

    QRect recScreen;
};



class CInfoShow : public QMainWindow
{
    Q_OBJECT
public:
    CInfoShow(QWidget *parent = nullptr);
    ~CInfoShow();

    void show();
    void hide();

private slots:
    void setupUI();

private:
    QLabel* m_pLabel;
    QRect recScreen;
};





#endif // CIMAGE_H
