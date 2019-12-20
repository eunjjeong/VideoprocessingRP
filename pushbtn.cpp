#include "videoprocessing.h"
#include "ui_videoprocessing.h"
#include <wiringPi.h>

using namespace cv;
using namespace std;

void Videoprocessing::on_ccwBtn_pressed()
{
    ui->rotate_view->fitInView(&pixmapRT, Qt::KeepAspectRatio);
    ui->rotate_view->rotate(-10);
}

void Videoprocessing::on_cwBtn_pressed()
{
    ui->rotate_view->fitInView(&pixmapRT, Qt::KeepAspectRatio);
    ui->rotate_view->rotate(10);
}

void Videoprocessing::on_autoBtn_pressed()
{
    ui->rotate_view->resetMatrix();
}

void Videoprocessing::on_upBtn_clicked()
{
    ui->move_view->move(ui->move_view->x(), ui->move_view->y()-10);
}

void Videoprocessing::on_leftBtn_clicked()
{
    ui->move_view->move(ui->move_view->x()-10, ui->move_view->y());
}

void Videoprocessing::on_rightBtn_clicked()
{
    ui->move_view->move(ui->move_view->x()+10, ui->move_view->y());
}

void Videoprocessing::on_down_clicked()
{
    ui->move_view->move(ui->move_view->x(), ui->move_view->y()+10);
}

void Videoprocessing::on_automvBtn_clicked()
{
    ui->move_view->move(170, 120);
    ui->move_view->resize(161, 131);
}

void Videoprocessing::on_zoomP_clicked()
{
    ui->move_view->resize(ui->move_view->width()+12, ui->move_view->height()+12);
    ui->move_view->move(ui->move_view->x()-6, ui->move_view->y()-6);
}

void Videoprocessing::on_zoomM_clicked()
{
    ui->move_view->resize(ui->move_view->width()-12, ui->move_view->height()-12);
    ui->move_view->move(ui->move_view->x()+6, ui->move_view->y()+6);
}

void Videoprocessing::on_recordBtn_pressed()
{
    ui->recordBtn->setText("STOP");
    string rec_path = ui->rec_path->text().toStdString();

    Size size = Size((int)video.get(CAP_PROP_FRAME_WIDTH), (int)video.get(CAP_PROP_FRAME_HEIGHT));
    int fourcc = VideoWriter::fourcc('H','2','6','4');
    double fps = 10;
    int delay = cvRound(1000.0 / fps);

    VideoWriter writer = VideoWriter(rec_path, fourcc, fps, size);

    wiringPiSetup();
    pinMode(25, OUTPUT);

    while(1) {
        Mat frame_rec;
        video >> frame_rec;
        writer << frame_rec;

        imshow("REC", frame_rec);
        //led
        digitalWrite(25, 1);
        if(waitKey(delay) >= 0) {
            digitalWrite(25, 0);
            break;
        }
    }
    writer.release();
    destroyWindow("REC");
    ui->recordBtn->setText("Record");
    cout << "REC END" << endl;
}

void Videoprocessing::on_captureBtn_clicked()
{
    wiringPiSetup();
    pinMode(25, OUTPUT);
    string cap_path = ui->capture_path->text().toStdString();
    Mat frame_cap;
    video >> frame_cap;

    imwrite(cap_path, frame_cap);
    Mat img = imread(cap_path);

    digitalWrite(25, 0);
    imshow("capture", img);
    waitKey(0);
    digitalWrite(25, 0);
    destroyWindow("image");
}

