#ifndef VIDEOPROCESSING_H
#define VIDEOPROCESSING_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QMessageBox>
#include <QCloseEvent>
#include <QDebug>
#include <QPixmap>
#include <QImage>

#include "opencv2/opencv.hpp"
#include <opencv2/highgui.hpp>
#include "opencv2/objdetect.hpp"

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/core/types_c.h>

QT_BEGIN_NAMESPACE
namespace Ui { class Videoprocessing; }
QT_END_NAMESPACE

class Videoprocessing : public QMainWindow
{
    Q_OBJECT

public:
    Videoprocessing(QWidget *parent = nullptr);
    ~Videoprocessing();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void on_startBtn_pressed();

    void on_ccwBtn_pressed();
    void on_cwBtn_pressed();
    void on_autoBtn_pressed();

    void on_upBtn_clicked();
    void on_leftBtn_clicked();
    void on_rightBtn_clicked();
    void on_down_clicked();
    void on_automvBtn_clicked();
    void on_zoomP_clicked();
    void on_zoomM_clicked();

    void on_recordBtn_pressed();
    void on_captureBtn_clicked();

private:
    Ui::Videoprocessing *ui;
    cv::VideoCapture video;

    QGraphicsPixmapItem pixmapR;
    QGraphicsPixmapItem pixmapG;
    QGraphicsPixmapItem pixmapB;
    QGraphicsPixmapItem pixmapREC;
    QGraphicsPixmapItem pixmapRT;
    QGraphicsPixmapItem pixmapMV;
    QGraphicsPixmapItem pixmapRGB;

    QGraphicsPixmapItem pixmapOG;
    QGraphicsPixmapItem pixmapOG2;
    QGraphicsPixmapItem pixmapRC;
    QGraphicsPixmapItem pixmapGC;
    QGraphicsPixmapItem pixmapBC;

    QGraphicsPixmapItem pixmapBlur;
    QGraphicsPixmapItem pixmapGray;
    QGraphicsPixmapItem pixmapED;

    QGraphicsPixmapItem pixmapfaceD;
    QGraphicsPixmapItem pixmapcarD;
    QGraphicsPixmapItem pixmapnumD;

    QGraphicsPixmapItem pixmapTV;
    QGraphicsPixmapItem pixmapTV2;

    QGraphicsPixmapItem pixmapSEN;
};
#endif // VIDEOPROCESSING_H
