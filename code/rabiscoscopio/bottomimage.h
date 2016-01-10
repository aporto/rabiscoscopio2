#ifndef BOTTOMIMAGE_H
#define BOTTOMIMAGE_H

#include <QWidget>
#include <QPainter>

#include "ui_bottomimage.h"
#include "definitions.h"

#include <vector>

using namespace std;

extern vector <TVecPoint> points;
extern vector <double> axis_x;
extern vector <double> axis_y;

class BottomImage : public QWidget
{
	Q_OBJECT

public:
	BottomImage(QWidget *parent = 0);
	~BottomImage();

private:
	Ui::BottomImage ui;

	void paintEvent(QPaintEvent *);
};

#endif // BOTTOMIMAGE_H
