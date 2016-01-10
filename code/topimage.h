#ifndef TOPIMAGE_H
#define TOPIMAGE_H

#include <QWidget>
#include <QPainter>
#include <QRect>

#include "ui_topimage.h"

#include "definitions.h"

#include <vector>

using namespace std;

extern vector <TVecPoint> points;
extern vector <double> axis_x;
extern vector <double> axis_y;

class TopImage : public QWidget
{
	Q_OBJECT

public:
	TopImage(QWidget *parent = 0);
	~TopImage();

private:
	Ui::TopImage ui;

	void paintEvent(QPaintEvent *);

};

#endif // TOPIMAGE_H
