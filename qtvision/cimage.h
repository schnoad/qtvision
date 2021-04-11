#ifndef CIMAGE_H
#define CIMAGE_H

#include <QMainWindow>
#include <QTimer>

class QLabel;
class QHBoxLayout;

class CImage : public QMainWindow
{
    Q_OBJECT
public:
    CImage(QWidget *parent = nullptr);
    ~CImage();
    void changeBackground(QString i_sPath);

private slots:
    void setupUI();
    void timerSlot();

private:
    QTimer* m_pTimer;
    QLabel* m_pLabel;

    int m_nImageCount;
};








#endif // CIMAGE_H
