#include "mylabel.h"

MyLabel::MyLabel(QWidget *parent):QLabel(parent)
{
    this->isPressed = false;
    this->EndPoint = QPoint(0,0);
    this->StartPoint = QPoint(0,0);
}

MyLabel::~MyLabel()
{

}


void MyLabel::paintEvent(QPaintEvent *event)
{
    QLabel::paintEvent(event);
    QPainter painter(this);
    painter.setPen(QPen(Qt::blue, 1, Qt::SolidLine, Qt::FlatCap));
    if (isPressed)
    {
        bbox_w = EndPoint.x()-StartPoint.x();
        bbox_h = EndPoint.y()-StartPoint.y();
        QRectF rectangle(StartPoint.x(), StartPoint.y(), bbox_w,bbox_h);

        //qDebug()<< StartPoint.x()<<EndPoint.y();
        painter.drawRect(rectangle);
    }

    painter.end();
}

//鼠标按下
void MyLabel::mousePressEvent(QMouseEvent *e)
{
    StartPoint = e->pos();
    EndPoint = e->pos();
    //在图片上绘制
    isPressed = true;
}

//鼠标移动
void MyLabel::mouseMoveEvent(QMouseEvent *e)
{
    if(isPressed)
    {
        EndPoint=e->pos();
        update();
    }
}

//鼠标抬起
void MyLabel::mouseReleaseEvent(QMouseEvent *e)
{


    EndPoint=e->pos();
    isPressed=false;
    update();
    qDebug()<< StartPoint.x()<<StartPoint.y()<<bbox_w<<bbox_h;
    emit emitbboox(StartPoint,bbox_w,bbox_h);

}

void MyLabel::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)
    {
        isPressed=false;
       qDebug()<< StartPoint.x()<<StartPoint.y()<<bbox_w<<bbox_h;
       emit emitbboox(StartPoint,bbox_w,bbox_h);
       MyLabel::clear();

    }
}

void MyLabel::clear()
{
    this->EndPoint = QPoint(0,0);
    this->StartPoint = QPoint(0,0);
    update();
}

void MyLabel::slotGetOneFrame(QImage img)
{

}
