#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QLabel>
#include <QPainter>
#include <QSound>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QFileSystemWatcher>
#include "ui_mainwindow.h"
#include "topimage.h"
#include "bottomimage.h"
#include "aboutwin.h"

#include <fstream>
#include <vector>

#include "definitions.h"

#pragma pack(push, 1)

struct TWaveHeader {
	char chunkId[4];
	unsigned int chunkSize;
	char format[4];
	char subChunk1Id[4];
	unsigned int subChunk1Size;
	unsigned short audioFormat;
	unsigned short numChannels;
	unsigned int sampleRate;
	unsigned int byteRate;
	unsigned short blockAlign;
	unsigned short bitsPerSample;
	char subChunk2Id[4];
	unsigned int subChunk2Size;
};

#pragma pack(pop)

using namespace std;

extern vector <TVecPoint> points;
extern vector <double> axis_x;
extern vector <double> axis_y;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();

public slots:
	void OnLoadFile();
	void OnRefresh();
	void OnAutoPlayClick();
	void OnAboutClick();
	void OncbUseOnlyXComponents(int state);
	
	void OnFileChanged(const QString &path);
	//void OnDirectoryChanged(const QString & path)

private:
	Ui::MainWindowClass ui;
	TopImage * topImage;
	BottomImage * bottomImage;
	
	double sampleRate;
	double duration;
	double fileLength;
	double xCompensate, yCompensate;
	double zoom;

	QString filename;
	QString wavefile;

	QSound * wave;
	
	QFileSystemWatcher * fileMonitor;
	void DecodeLine2(QString line, bool relative, TVecPoint & lastPoint);
	void DecodeLine(QString line, bool relative);
	void NormalizePoints();
	void BreakAxis();
	void WriteWaveFile();
	double distance(TVecPoint p1, TVecPoint p2);

};

#endif // MAINWINDOW_H
