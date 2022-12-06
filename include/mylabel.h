 #ifndef MYLABEL_H
#define MYLABEL_H
#include <list>
#include <map>
#include<QLabel>
#include<QPoint>
#include<QMouseEvent>
#include<QPainter>
#include<QWidget>
#include<QDebug>
#include<iostream>
#include<QString>
#include<QImage>


using namespace std;



class MyLabel : public QLabel
{
     Q_OBJECT
public:

    MyLabel(QWidget *parent = nullptr);
    ~MyLabel();


    int x,y,w,h;
    //鼠标按下
    void mousePressEvent(QMouseEvent *e);
    //鼠标移动
    void mouseMoveEvent(QMouseEvent *e);
    //鼠标抬起
    void mouseReleaseEvent(QMouseEvent *e);

    void keyPressEvent(QKeyEvent *e);

    void clear();


protected:
    void  paintEvent(QPaintEvent *event);

signals:
    void emitbboox(QPoint start,int w,int h);


private slots :
    void  slotGetOneFrame(QImage img);

private:
    QImage wb_Image;
    QPoint StartPoint;
    QPoint EndPoint;
    int bbox_w;
    int bbox_h;
    bool isPressed;
};

#endif // MYLABEL_H
