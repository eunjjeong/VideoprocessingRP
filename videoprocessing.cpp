#include "videoprocessing.h"
#include "ui_videoprocessing.h"
#include <wiringPi.h>
#include <wiringPiSPI.h>

#define MCP3008 28
#define SPI_CHANNEL 0
#define SPI_SPEED 1000000

using namespace cv;
using namespace std;

int threshold1 = 30;
Vec3b lower_blue1, upper_blue1, lower_blue2, upper_blue2, lower_blue3, upper_blue3;
Mat img_color;


Videoprocessing::Videoprocessing(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Videoprocessing)
{
    ui->setupUi(this);

    ui->red_view->setScene(new QGraphicsScene(this));
    ui->red_view->scene()->addItem(&pixmapR);

    ui->green_view->setScene(new QGraphicsScene(this));
    ui->green_view->scene()->addItem(&pixmapG);

    ui->blue_view->setScene(new QGraphicsScene(this));
    ui->blue_view->scene()->addItem(&pixmapB);

    ui->rec_view->setScene(new QGraphicsScene(this));
    ui->rec_view->scene()->addItem(&pixmapREC);

    ui->rotate_view->setScene(new QGraphicsScene(this));
    ui->rotate_view->scene()->addItem(&pixmapRT);

    ui->move_view->setScene(new QGraphicsScene(this));
    ui->move_view->scene()->addItem(&pixmapMV);

    ui->orig_view->setScene(new QGraphicsScene(this));
    ui->orig_view->scene()->addItem(&pixmapOG2);

    ui->redC_view->setScene(new QGraphicsScene(this));
    ui->redC_view->scene()->addItem(&pixmapRC);

    ui->greenC_view->setScene(new QGraphicsScene(this));
    ui->greenC_view->scene()->addItem(&pixmapGC);

    ui->blueC_view->setScene(new QGraphicsScene(this));
    ui->blueC_view->scene()->addItem(&pixmapBC);

    ui->orig_view2->setScene(new QGraphicsScene(this));
    ui->orig_view2->scene()->addItem(&pixmapOG);

    ui->blur_view->setScene(new QGraphicsScene(this));
    ui->blur_view->scene()->addItem(&pixmapBlur);

    ui->gray_view->setScene(new QGraphicsScene(this));
    ui->gray_view->scene()->addItem(&pixmapGray);

    ui->edge_view->setScene(new QGraphicsScene(this));
    ui->edge_view->scene()->addItem(&pixmapED);

    ui->faceD_view->setScene(new QGraphicsScene(this));
    ui->faceD_view->scene()->addItem(&pixmapfaceD);

    ui->carD_view->setScene(new QGraphicsScene(this));
    ui->carD_view->scene()->addItem(&pixmapcarD);

    ui->numD_view->setScene(new QGraphicsScene(this));
    ui->numD_view->scene()->addItem(&pixmapnumD);

    ui->track_view->setScene(new QGraphicsScene(this));
    ui->track_view->scene()->addItem(&pixmapTV);

    ui->trackC_view->setScene(new QGraphicsScene(this));
    ui->trackC_view->scene()->addItem(&pixmapTV2);

    ui->sensor_view->setScene(new QGraphicsScene(this));
    ui->sensor_view->scene()->addItem(&pixmapSEN);
}


//histogram
void calc_Histo(const Mat& frame, Mat& hist, int bins, int range_max = 256)
{
    int histSize[] = {bins};
    float range[] = {0, (float)range_max};
    int channels[] = {0};
    const float* ranges[] = {range};

    calcHist(&frame, 1, channels, Mat(), hist, 1, histSize, ranges);
}

Mat color_histo(int rows, int num)
{
    Mat c_his(rows, 1, CV_8UC3);

    for (int i = 0; i < rows; i++) {
        if(num == 1){
            c_his.at<Vec3b>(i) = Vec3b(0, 0, 255);
        }
        else if(num == 2){
            c_his.at<Vec3b>(i) = Vec3b(0, 255, 0);
        }
        else if(num == 3){
            c_his.at<Vec3b>(i) = Vec3b(255, 0, 0);
        }
    }
    cvtColor(c_his, c_his, COLOR_BGR2RGB);
    return c_his;
}

void draw_histo(Mat hist, Mat& hist_frame, Size size = Size(256, 200), int a=0) {

    Mat his_color = color_histo(hist.rows, a);

    hist_frame = Mat(size, CV_8UC3, Scalar(255, 255, 255));
    float bin = (float)hist_frame.cols / hist.rows;

    normalize(hist, hist, 0, hist_frame.rows, NORM_MINMAX);

    for(int i = 0; i < hist.rows; i++) {
        float start_x = i * bin;
        float end_x = (i + 1) * bin;
        Point2f pt1(start_x, 0);
        Point2f pt2(end_x, hist.at<float>(i));
        Scalar color = his_color.at<Vec3b>(i);
        if(pt2.y > 0) {
            rectangle(hist_frame, pt1, pt2, color, -1);
        }
    }
    flip(hist_frame, hist_frame, 0);
}

Mat histogram(Mat frame, int num){
    Mat bgr[3];

    split(frame, bgr);
    Mat rgb_hist, frame_his;

    if(num == 1){
        calc_Histo(bgr[0], rgb_hist, 256, 256); // red
        draw_histo(rgb_hist, frame_his, Size(360, 200), 1);
       }
    else if(num == 2)
       {
        calc_Histo(bgr[1], rgb_hist, 256, 256); // green
        draw_histo(rgb_hist, frame_his, Size(360, 200), 2);
       }
    else if(num == 3)
       {
        calc_Histo(bgr[2], rgb_hist, 256, 256); // blue
        draw_histo(rgb_hist, frame_his, Size(360, 200), 3);
       }
    return frame_his;
}


// tracking
void mouse_callback(int event, int x, int y, int flags, void *param)
{
    if (event == EVENT_LBUTTONDOWN)
    {
        Vec3b color_pixel = img_color.at<Vec3b>(y, x);

        Mat bgr_color = Mat(1, 1, CV_8UC3, color_pixel);


        Mat hsv_color;
        cvtColor(bgr_color, hsv_color, COLOR_BGR2HSV);

        int hue = hsv_color.at<Vec3b>(0, 0)[0];
        int saturation = hsv_color.at<Vec3b>(0, 0)[1];
        int value = hsv_color.at<Vec3b>(0, 0)[2];


        if (hue < 10)
        {
            cout << "case 1" << endl;
            lower_blue1 = Vec3b(hue - 10 + 180, threshold1, threshold1);
            upper_blue1 = Vec3b(180, 255, 255);
            lower_blue2 = Vec3b(0, threshold1, threshold1);
            upper_blue2 = Vec3b(hue, 255, 255);
            lower_blue3 = Vec3b(hue, threshold1, threshold1);
            upper_blue3 = Vec3b(hue+10, 255, 255);
        }
        else if (hue > 170)
        {
            cout << "case 2" << endl;
            lower_blue1 = Vec3b(hue, threshold1, threshold1);
            upper_blue1 = Vec3b(180, 255, 255);
            lower_blue2 = Vec3b(0, threshold1, threshold1);
            upper_blue2 = Vec3b(hue + 10 - 180, 255, 255);
            lower_blue3 = Vec3b(hue - 10, threshold1, threshold1);
            upper_blue3 = Vec3b(hue, 255, 255);
        }
        else
        {
            cout << "case 3" << endl;
            lower_blue1 = Vec3b(hue, threshold1, threshold1);
            upper_blue1 = Vec3b(hue + 10, 255, 255);
            lower_blue2 = Vec3b(hue - 10, threshold1, threshold1);
            upper_blue2 = Vec3b(hue, 255, 255);
            lower_blue3 = Vec3b(hue - 10, threshold1, threshold1);
            upper_blue3 = Vec3b(hue, 255, 255);
        }
    }
}

// face detect
string cascadeName, nestedCascadeName;
Mat detectAndDraw( Mat& img, CascadeClassifier& cascade, CascadeClassifier& nestedCascade, double scale)
{
    vector<Rect> faces, faces2;
    Mat gray, smallImg;

    cvtColor( img, gray, COLOR_BGR2GRAY ); // Convert to Gray Scale
    double fx = 1 / scale;

    // Resize the Grayscale Image
    resize( gray, smallImg, Size(), fx, fx, INTER_LINEAR );
    equalizeHist( smallImg, smallImg );

    // Detect faces of different sizes using cascade classifier
    cascade.detectMultiScale( smallImg, faces, 1.1,
                            2, 0|CASCADE_SCALE_IMAGE, Size(30, 30) );

    // Draw circles around the faces
    for ( size_t i = 0; i < faces.size(); i++ )
    {
        Rect r = faces[i];
        Mat smallImgROI;
        vector<Rect> nestedObjects;
        Point center;
        Scalar color = Scalar(0, 255, 0); // Color for Drawing tool
        int radius;

        double aspect_ratio = (double)r.width/r.height;
        if( 0.75 < aspect_ratio && aspect_ratio < 1.3 )
        {
            center.x = cvRound((r.x + r.width*0.5)*scale);
            center.y = cvRound((r.y + r.height*0.5)*scale);
            radius = cvRound((r.width + r.height)*0.25*scale);
            circle( img, center, radius, color, 3, 8, 0 );
        }
        else
            rectangle(img, cvPoint(cvRound(r.x*scale), cvRound(r.y*scale)),
                    cvPoint(cvRound((r.x + r.width-1)*scale),
                    cvRound((r.y + r.height-1)*scale)), color, 3, 8, 0);
        if( nestedCascade.empty() )
            continue;
        smallImgROI = smallImg( r );

        // Detection of eyes int the input image
        nestedCascade.detectMultiScale( smallImgROI, nestedObjects, 1.1, 2,
                                        0|CASCADE_SCALE_IMAGE, Size(30, 30) );

        // Draw circles around eyes
        for ( size_t j = 0; j < nestedObjects.size(); j++ )
        {
            Rect nr = nestedObjects[j];
            center.x = cvRound((r.x + nr.x + nr.width*0.5)*scale);
            center.y = cvRound((r.y + nr.y + nr.height*0.5)*scale);
            radius = cvRound((nr.width + nr.height)*0.25*scale);
            circle( img, center, radius, color, 3, 8, 0 );
        }
    }
    return img;
}

//sensor
int adcRead(unsigned char adcChannel){
    unsigned char buff[3];
    int adcValue = 0;

    buff[0] = 0x01;
    buff[1] = 0x80|((adcChannel & 0x07) << 4);
    buff[2] = 0x00;

    digitalWrite(MCP3008, 0);
    wiringPiSPIDataRW(SPI_CHANNEL, buff, 3);

    buff[1] = 0x03 & buff[1];
    adcValue = (buff[1]<<8) | buff[2];
    digitalWrite(MCP3008, 1);

    return adcValue;
}


void Videoprocessing::on_startBtn_pressed()
{
    if(video.isOpened())
    {
        ui->startBtn->setText("Start");
        video.release();
        return;
    }

    bool isCamera;
    int cameraIndex = ui->videoEdit->text().toInt(&isCamera);
    if(isCamera)
    {
        if(!video.open(cameraIndex))
        {
            QMessageBox::critical(this, "Camera Error", "Make sure you entered a correct camera index," "<br>or that the camera is not being accessed by another program!");
            return;
        }
    }
    else
    {
        if(!video.open(ui->videoEdit->text().trimmed().toStdString()))
        {
            QMessageBox::critical(this, "Video Error", "Make sure you entered a correct and supported video file path," "<br>or a correct RTSP feed URL!");
            return;
        }
    }

    ui->startBtn->setText("Stop");

    Mat frame, frame_gray;

    video.set(3, 640);
    video.set(4, 480);

    while(video.isOpened())
    {
        video >> frame;

            CV_Assert(frame.data);
            cvtColor(frame, frame_gray, COLOR_BGR2GRAY);
            Mat r_hist, g_hist, b_hist, frame_blur, frame_edge;

            // historgram (rgb)
            r_hist = histogram(frame, 1);
            g_hist = histogram(frame, 2);
            b_hist = histogram(frame, 3);

            // blur, cannyEdge
            GaussianBlur(frame, frame_blur, Size(55, 55), 0);
            Canny(frame, frame_edge, 100, 150);

            // red hist
            QImage qimgR(r_hist.data, r_hist.cols, r_hist.rows, r_hist.step, QImage::Format_RGB888);
            pixmapR.setPixmap(QPixmap::fromImage(qimgR));
            ui->red_view->fitInView(&pixmapR, Qt::IgnoreAspectRatio);

            // green hist
            QImage qimgG(g_hist.data, g_hist.cols, g_hist.rows, g_hist.step, QImage::Format_RGB888);
            pixmapG.setPixmap(QPixmap::fromImage(qimgG));
            ui->green_view->fitInView(&pixmapG, Qt::IgnoreAspectRatio);

            // blue hist
            QImage qimgB(b_hist.data, b_hist.cols, b_hist.rows, b_hist.step, QImage::Format_RGB888);
            pixmapB.setPixmap(QPixmap::fromImage(qimgB));
            ui->blue_view->fitInView(&pixmapB, Qt::IgnoreAspectRatio);

        if(ui->tabWidget->currentIndex() == 0){
            // RGB channel
            QImage qimgOrig2(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
            pixmapOG2.setPixmap(QPixmap::fromImage(qimgOrig2.rgbSwapped()));
            ui->orig_view->fitInView(&pixmapOG2, Qt::IgnoreAspectRatio);

            Mat rgb[3];
            Mat zFillMatrix = Mat::zeros(frame.size(), CV_8UC1);
            split(frame, rgb);
            Mat R[] = {zFillMatrix, zFillMatrix, rgb[2]};
            Mat G[] = {zFillMatrix, rgb[1], zFillMatrix};
            Mat B[] = {rgb[0], zFillMatrix, zFillMatrix};
            merge(R, 3, rgb[2]);
            merge(G, 3, rgb[1]);
            merge(B, 3, rgb[0]);

            // RGB channel(RED)
            QImage qimgRED(rgb[2].data, rgb[2].cols, rgb[2].rows, rgb[2].step, QImage::Format_RGB888);
            pixmapRC.setPixmap(QPixmap::fromImage(qimgRED.rgbSwapped()));
            ui->redC_view->fitInView(&pixmapRC, Qt::IgnoreAspectRatio);

            // RGB channel(GREEN)
            QImage qimgGREEN(rgb[1].data, rgb[1].cols, rgb[1].rows, rgb[1].step, QImage::Format_RGB888);
            pixmapGC.setPixmap(QPixmap::fromImage(qimgGREEN.rgbSwapped()));
            ui->greenC_view->fitInView(&pixmapGC, Qt::IgnoreAspectRatio);

            // RGB channel(BLUE)
            QImage qimgBLUE(rgb[0].data, rgb[0].cols, rgb[0].rows, rgb[0].step, QImage::Format_RGB888);
            pixmapBC.setPixmap(QPixmap::fromImage(qimgBLUE.rgbSwapped()));
            ui->blueC_view->fitInView(&pixmapBC, Qt::IgnoreAspectRatio);
        }

        if(ui->tabWidget->currentIndex() == 1){
            // REC
            QImage qimgREC(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
            pixmapREC.setPixmap(QPixmap::fromImage(qimgREC.rgbSwapped()));
            ui->rec_view->fitInView(&pixmapREC, Qt::IgnoreAspectRatio);
        }

        if(ui->tabWidget->currentIndex() == 2){
            // ROTATE
            QImage qimgRT(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
            pixmapRT.setPixmap(QPixmap::fromImage(qimgRT.rgbSwapped()));
            ui->rotate_view->fitInView(&pixmapRT, Qt::IgnoreAspectRatio);
        }

        if(ui->tabWidget->currentIndex() == 3){
            // MOVE
            QImage qimgMV(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
            pixmapMV.setPixmap(QPixmap::fromImage(qimgMV.rgbSwapped()));
            ui->move_view->fitInView(&pixmapMV, Qt::IgnoreAspectRatio);
        }

        if(ui->tabWidget->currentIndex() == 4){
            Mat grayframe;
            //gray scale로 변환
            cvtColor(frame, grayframe, COLOR_BGR2GRAY);
            //histogram 얻기
            equalizeHist(grayframe, grayframe);

                // Face Detect
                if(ui->tabWidget_2->currentIndex() == 0){
                CascadeClassifier classifier, classifier2;
                double scale=1;
                Mat image;
                classifier.load("haarcascade_frontalface_default.xml");
                //classifier2.load("haarcascade_eye_tree_eyeglasses.xml");

                Mat frame1 = grayframe.clone();
                detectAndDraw(frame1, classifier, classifier2, scale);

                QImage qimgFace(frame1.data, frame1.cols, frame1.rows, frame1.step, QImage::Format_RGB888);
                pixmapfaceD.setPixmap(QPixmap::fromImage(qimgFace.rgbSwapped()));
                ui->faceD_view->fitInView(&pixmapfaceD, Qt::IgnoreAspectRatio);
                }

                // Car Detect
                if(ui->tabWidget_2->currentIndex() == 1){
                CascadeClassifier classifier;
                classifier.load("cars.xml");
                vector<Rect> detecs;
                classifier.detectMultiScale(grayframe, detecs);
                for(Rect rc : detecs) {
                    rectangle(frame, rc, Scalar(0, 255, 0), 2);
                }
                QImage qimgCar(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
                pixmapcarD.setPixmap(QPixmap::fromImage(qimgCar.rgbSwapped()));
                ui->carD_view->fitInView(&pixmapcarD, Qt::IgnoreAspectRatio);
                }

                // Number Detect
                if(ui->tabWidget_2->currentIndex() == 2){
                CascadeClassifier classifier;
                classifier.load("haarcascade_russian_plate_number.xml");
                vector<Rect> detecs;
                classifier.detectMultiScale(grayframe, detecs);
                for(Rect rc : detecs) {
                    rectangle(frame, rc, Scalar(0, 255, 0), 2);
                }
                QImage qimgNum(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
                pixmapnumD.setPixmap(QPixmap::fromImage(qimgNum.rgbSwapped()));
                ui->numD_view->fitInView(&pixmapnumD, Qt::IgnoreAspectRatio);
                }
         }

        if(ui->tabWidget->currentIndex() == 5){
            // filter
            QImage qimgOrig(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
            pixmapOG.setPixmap(QPixmap::fromImage(qimgOrig.rgbSwapped()));
            ui->orig_view2->fitInView(&pixmapOG, Qt::IgnoreAspectRatio);

            QImage qimgBlur(frame_blur.data, frame_blur.cols, frame_blur.rows, frame_blur.step, QImage::Format_RGB888);
            pixmapBlur.setPixmap(QPixmap::fromImage(qimgBlur.rgbSwapped()));
            ui->blur_view->fitInView(&pixmapBlur, Qt::IgnoreAspectRatio);

            QImage qimgGray(frame_gray.data, frame_gray.cols, frame_gray.rows, frame_gray.step, QImage::Format_Grayscale8);
            pixmapGray.setPixmap(QPixmap::fromImage(qimgGray.rgbSwapped()));
            ui->gray_view->fitInView(&pixmapGray, Qt::IgnoreAspectRatio);

            QImage qimgEdge(frame_edge.data, frame_edge.cols, frame_edge.rows, frame_edge.step, QImage::Format_Grayscale8);
            pixmapED.setPixmap(QPixmap::fromImage(qimgEdge.rgbSwapped()));
            ui->edge_view->fitInView(&pixmapED, Qt::IgnoreAspectRatio);
        }
        if(ui->tabWidget->currentIndex() == 6){
            //sensor
            Mat frameSEN;

            wiringPiSetup();
            pinMode(MCP3008, OUTPUT);
            wiringPiSPISetup(SPI_CHANNEL, SPI_SPEED);

            int adcChannel = 3;
            int adcValue = adcRead(adcChannel);
            delay(300);
            adcValue = adcValue;

            frame.convertTo(frameSEN, -1, 1, adcValue);
            QImage qimgSEN(frameSEN.data, frameSEN.cols, frameSEN.rows, frameSEN.step, QImage::Format_RGB888);
            pixmapSEN.setPixmap(QPixmap::fromImage(qimgSEN.rgbSwapped()));
            ui->sensor_view->fitInView(&pixmapSEN, Qt::IgnoreAspectRatio);
            ui->sennum->display(adcValue);
        }

        if(ui->tabWidget->currentIndex() == 7){
            // tracking
            namedWindow("img_color");
            setMouseCallback("img_color", mouse_callback);

            Mat img_hsv, frame_track;

            while(1){
            video >> frame_track;
            video.read(img_color);
            cvtColor(img_color, img_hsv, COLOR_BGR2HLS);

            Mat img_mask1, img_mask2, img_mask3, img_mask;
            inRange(img_hsv, lower_blue1, upper_blue1, img_mask1);
            inRange(img_hsv, lower_blue2, upper_blue2, img_mask2);
            inRange(img_hsv, lower_blue3, upper_blue3, img_mask3);
            img_mask = img_mask1 | img_mask2 | img_mask3;

            Mat img_result;
            bitwise_and(img_color, img_color, img_result, img_mask);

            imshow("img_color", img_color);
            QImage qimgTrack2(img_color.data, img_color.cols, img_color.rows, img_color.step, QImage::Format_RGB888);
            pixmapTV2.setPixmap(QPixmap::fromImage(qimgTrack2.rgbSwapped()));
            ui->trackC_view->fitInView(&pixmapTV2, Qt::IgnoreAspectRatio);

            QImage qimgTrack(img_result.data, img_result.cols, img_result.rows, img_result.step, QImage::Format_RGB888);
            pixmapTV.setPixmap(QPixmap::fromImage(qimgTrack.rgbSwapped()));
            ui->track_view->fitInView(&pixmapTV, Qt::IgnoreAspectRatio);

            if (waitKey(1)>0)
                break;
            }
        }

        qApp->processEvents();
    }
    ui->startBtn->setText("Start");
}

void Videoprocessing::closeEvent(QCloseEvent *event)
{
    if(video.isOpened())
    {
        QMessageBox::warning(this, "Warning", "Stop the video before closing the application!");
        event->ignore();
    }
    else
    {
        event->accept();
    }
}

Videoprocessing::~Videoprocessing()
{
    delete ui;
}

