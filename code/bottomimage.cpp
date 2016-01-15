#include "bottomimage.h"

BottomImage::BottomImage(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	hide();
}

BottomImage::~BottomImage()
{

}

void BottomImage::paintEvent(QPaintEvent *)
{
	 QPainter painter(this);
     //painter.setPen(Qt::blue);
     //painter.setFont(QFont("Arial", 30));
     //painter.drawText(rect(), Qt::AlignCenter, "Qt");
	
	double w = width();
	double h = height();
	//TCanvas * can = imgMain->Picture->Bitmap->Canvas;
	//imgMain->Picture->Bitmap->Width = w;
	//imgMain->Picture->Bitmap->Height = h;

	painter.setPen(Qt::black);
	painter.setBrush(Qt::black);
	QRect rectangle(0,0,w,h);
	painter.drawRect(rectangle);

	painter.setPen(Qt::darkGreen);
	painter.drawLine(QLine(0, h/2, w, h/2));
	painter.drawLine(QLine(w/2, 0, w/2, h));

	painter.setBrush(Qt::NoBrush);
	// axis y
	painter.setPen(Qt::blue);
	double x,y;
	if (axis_y.size() > 0) {
		y = axis_y.at(0);
		y = y * h/200 + h/2;
		x = 0;
		QPainterPath path;
		path.moveTo(x, y);
		for (unsigned int i = 1; i < axis_y.size(); i++) {
			y = axis_y.at(i);
			y = h - (y * h/2 + h/2);
			//y = h - (y * h/200 + h/2);
			x = i * w / axis_y.size();
			path.lineTo(x, y);
		}
		painter.drawPath(path);
	}


	// axis x
	QPen pen(Qt::yellow);
	pen.setStyle(Qt::DashLine);
	painter.setPen(pen);
	if (axis_x.size() > 0) {
		y = axis_x.at(0);
		y = y * h/200 + h/2;
		x = 0;
		QPainterPath path;
		path.moveTo(x, y);
		for (unsigned int i = 1; i < axis_x.size(); i++) {
			y = axis_x.at(i);
			//y = h - (y * h/200 + h/2);
			y = h - (y * h/2 + h/2);
			x = i * w / axis_x.size();
			path.lineTo(x, y);
		}
		painter.drawPath(path);
	}

	painter.setPen(Qt::yellow);
	painter.setFont(QFont("Arial", 12));
	painter.drawText(rect(), Qt::AlignLeft, "X-axis");

	painter.setPen(Qt::blue);
	painter.setFont(QFont("Arial", 12));
	painter.drawText(rect(), Qt::AlignRight, "Y-axis");
}
