#include "mainwindow.h"

vector <TVecPoint> points;
vector <double> axis_x;
vector <double> axis_y;

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	QSplitter *splitter = new QSplitter(Qt::Vertical);
	ui.verticalLayout->addWidget(splitter);
	splitter->show();
	//splitter->
	topImage  = new TopImage();
	bottomImage  = new BottomImage();	
	//QTextEdit *textedit = new QTextEdit;
	splitter->addWidget(topImage);
	splitter->addWidget(bottomImage);
	topImage->show();
	bottomImage->show();

	QSplitterHandle *handle = splitter->handle(1);
	QVBoxLayout *layout = new QVBoxLayout(handle);
	layout->setSpacing(0);
	layout->setMargin(0);

	QFrame *line = new QFrame(handle);
	line->setFrameShape(QFrame::HLine);
	line->setFrameShadow(QFrame::Sunken);
	layout->addWidget(line);
	//splitter->addWidget(textedit);

	connect(ui.actionLoad, SIGNAL(triggered()), this, SLOT(OnLoadFile()));
	connect(ui.actionRefresh, SIGNAL(triggered()), this, SLOT(OnRefresh()));
	connect(ui.actionAutoPlay, SIGNAL(triggered()), this, SLOT(OnAutoPlayClick()));
	connect(ui.actionAbout, SIGNAL(triggered()), this, SLOT(OnAboutClick()));

	ui.progress->hide();	


	sampleRate = 48000;
	duration = 0.02;
	fileLength = 1;

	filename = "C:\\git_hub\\aporto\\rabiscoscopio2\\images\\star1.svg";

	wave = new QSound("", this);
	wave->setLoops(QSound::Infinite);

	fileMonitor = new QFileSystemWatcher (this);
	connect(fileMonitor, SIGNAL(fileChanged(QString)), this, SLOT(OnFileChanged(QString)));

}

MainWindow::~MainWindow()
{
	
}

void MainWindow::OnLoadFile()	
{
	QString oldFileName = filename;
	//QString fileName = QFileDialog::getOpenFileName(this,	tr("Open Image"), "", tr("SVG Image Files (*.svg)"));
	
	//QString fileName = "C:\\alex\\projetos\\github\\aporto\\rabiscoscopio2\\images\\garoa_logo.svg";
	filename = "C:/git_hub/aporto/rabiscoscopio2/images/star1.svg";

	bool b = fileMonitor->removePath(oldFileName);
	b = fileMonitor->addPath(filename);

	OnRefresh();	
}

void MainWindow::DecodeLine(QString line, bool relative)
{
	points.clear();
	line.remove(0,1);
	int p = line.indexOf(" ");
	bool first = true;
	TVecPoint last = {0, 0};
	while (line.size() > 0) {
		//QString mline = line.SubString(1, p-1);
		QString mline = line.left(p);
		line = line.remove(0, mline.size() + 1);

		int pv = mline.indexOf(",");
		int pe = mline.indexOf(" ");

		if (pv >= 0) {
			if (pe < pv) {
				pe = mline.size();
			}
			QString xs = mline.left(pv);
			QString ys = mline.left(pe);
			ys = ys.right(ys.size() - pv - 1);		

			//mline.remove(0, p

			TVecPoint point;
			try {
				point.x = xs.toDouble();
				point.y = ys.toDouble();
			} catch (...) {
				point.x = 0;
				point.y = 0;
			}

			if (relative) {
				point.x += last.x;
				point.y += last.y;
			}
			memcpy(&last, &point, sizeof(TVecPoint));

			points.push_back(point);
		} else {
			if (mline == "z") {
				points.push_back(points.at(0));
			}

			if (mline == "l") {
				relative = true;
			}

			if (mline == "L") {
				relative = true;
			}

			if (mline.toUpper() == mline) {
				relative = false;
			}
		}

		p = line.indexOf(" ");
		if (p < 0) {
			p = line.size();
		}
		first = false;
	}
}



void MainWindow::NormalizePoints()
{
	double minx = 10000000;
	double maxx = -10000000;
	double miny = 10000000;
	double maxy = -10000000;

	for (unsigned int i = 0; i < points.size(); i++) {
		if (points.at(i).x > maxx) {
			maxx = points.at(i).x;
		}
		if (points.at(i).y > maxy) {
			maxy = points.at(i).y;
		}
		if (points.at(i).x < minx) {
			minx = points.at(i).x;
		}
		if (points.at(i).y < miny) {
			miny = points.at(i).y;
		}
	}

	double w = maxx - minx;
	double h = maxy - miny;
	double zerox = w / 2;
	double zeroy = h / 2;

	for (unsigned int i = 0; i < points.size(); i++) {
		double x = points.at(i).x;
		double y = points.at(i).y;
		x = (x - minx) * 80 / w - 50 + 10;
		y = (y - miny) * 80 / h - 50 + 10;
		points.at(i).x = x;
		points.at(i).y = y;
	}
}

void MainWindow::BreakAxis()
{
	double totalx = 0;
	axis_x.clear();
	axis_y.clear();

	int psize = points.size()-1;

	// totalx holds the full length (sum of all "x" segments)
	for (int i = 0; i < psize; i++) {
		double delta;
		if (points.at(i).x > points.at(i+1).x) {
			delta = points.at(i).x - points.at(i+1).x;
		} else {
			delta = points.at(i+1).x - points.at(i).x;
		}
		totalx += delta;
	}

	// "times" hold all "x" values converted to real-time
	vector <double> times;
	times.push_back(0.0);
	for (int i = 0; i < psize; i++) {
		double delta;
		if (points.at(i).x > points.at(i+1).x) {
			delta = points.at(i).x - points.at(i+1).x;
		} else {
			delta = points.at(i+1).x - points.at(i).x;
		}
		double time = (delta/totalx) * duration + times.at(i);
		times.push_back(time);
	}

	unsigned int numberOfPoints = sampleRate * duration;
	int idx = 0;
	double timeStep = 1.0 / sampleRate;
	double time = 0.0;
	double grad = 0.0;
	double x1 = points.at(idx).x;
	double x2 = points.at(idx+1).x ;
	double y1 = points.at(idx).y;
	double y2 = points.at(idx+1).y;
	double lim_inf = times.at(idx);
	double lim_sup = times.at(idx+1);
	double x, y;
	int k = times.size();
	for (unsigned int i = 0; i < numberOfPoints; i++) {
		if (abs(lim_sup - lim_inf) < 0.00001) {
			grad = 0;
		} else {
			grad = ((time - lim_inf) / (lim_sup - lim_inf));
		}
		double delta = y2-y1;
		if (abs(delta) > 0.00001) {
			y = y1 + grad * delta;
		} else {
			y = y1;
		}
		delta = x2-x1;
		if (abs(delta) > 0.00001) {
			x = x1 + grad * delta;
		} else {
			x = x1;
		}
		//double x = points.at(idx + 1).x + grad * (points.at(idx + 1).x - points.at(i).x);

		axis_x.push_back(x);
		//axis_y.push_back(-(y));
		axis_y.push_back(-y);

		time = time + timeStep;
		if (time > lim_sup) {
			idx++;
			if (idx >= psize) {
				if (i < numberOfPoints - 10) {
					//ShowMessage("Ops! Deu pau na interpolacao!");
					QMessageBox::information(0, "error", "Interpolation error!");				
				}
				break;
			}
			x1 = points.at(idx).x;
			x2 = points.at(idx+1).x ;
			y1 = points.at(idx).y;
			y2 = points.at(idx+1).y;
			lim_inf = times.at(idx);
			lim_sup = times.at(idx+1);
		}
	}

	/*double d1 = axis_y.size();
	double d2 = sampleRate;
	double d3 = cycles;
	int t = (d1 / d2);
	lblPointCount->Caption = IntToStr(t);
	lblSmallerDeltaX->Caption = FloatToStrF(timeStep * 1000, ffFixed, 5, 3) + " ms";*/
}

//--------------------------------------------------------------------------------------------------

void MainWindow::OnRefresh()
{
	if (QFileInfo(filename).size() <= 0) {
		return;
	}

	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly)) {
		QMessageBox::information(0, "error", file.errorString());
	}

	QTextStream in(&file);

	QString line;

	bool found = false;

	while(!in.atEnd()) {
		line = in.readLine();
		if (line.contains("<path")) {
			found = true;
			break;
		}
	}

	if (found  == false) {
		QMessageBox::information(0, "error", "Invalid file format");
		return;
	}

	while(!in.atEnd()) {
		line += in.readLine();
		
	}

	file.close();

	int pos1 = line.indexOf("d=\"M");
	int pos2 = line.indexOf("d=\"m");

	if ((pos1 == -1) && (pos2 == -1)) {
		QMessageBox::information(0, "error", "Could not find single path on SVG file");
		return;
	}

	bool relative;
	if (pos1 < pos2) {
		if (pos1 > -1) {
			line.remove(0, pos1);
			relative = false;
		} else {
			line.remove(0, pos2);
			relative = true;
		}
	} else {
		if (pos2 > -1) {
			line.remove(0, pos2);
			relative = true;
		} else {
			line.remove(0, pos1);
			relative = false;
		}
	}

	line.remove(0, 4);

	pos1 = line.indexOf("\"");
	if (pos1 < 0) {
		QMessageBox::information(0, "error", "Could not find end of path on SVG file");
		return;
	}

	line.remove(pos1, line.size());

	DecodeLine(line, relative);
	NormalizePoints();
	BreakAxis();

	topImage->repaint();
	bottomImage->repaint();
	
	WriteWaveFile();	
}

void MainWindow::OnFileChanged(const QString &path)
{
	if (path == filename) {
		OnRefresh();
	} else {
		fileMonitor->removePath(path);
	}
}


void MainWindow::WriteWaveFile()
{
	QFileInfo info(filename);
	wavefile = info.path() + "\\" + info.completeBaseName() + ".wav";

	QFile::remove(wavefile);

	ui.progress->setValue(0);
	ui.progress->show();	

	unsigned int numSamples = 0;
	unsigned int dataSize = 0;
	TWaveHeader header;

	memcpy(header.chunkId, "RIFF", 4);
	memcpy(header.format, "WAVE", 4);

	memcpy(header.subChunk1Id, "fmt ", 4);
	header.subChunk1Size = 16;
	header.audioFormat = 1;
	header.numChannels = 2;
	header.sampleRate = sampleRate;
	header.bitsPerSample = 16;
	header.byteRate  = header.sampleRate * header.numChannels * (header.bitsPerSample/8);
	header.blockAlign = header.numChannels * (header.bitsPerSample/8);

	memcpy(header.subChunk2Id, "data", 4);

	char buffer[100];
	ofstream file (wavefile.toStdString().c_str(), ios::out | ios::binary);

	int sz = sizeof(TWaveHeader);
	file.write ((char *)&header, sz);
	int total = axis_y.size();
	int cycles = fileLength / duration;
	int pTotal = cycles * total;
	int prog = 0;
	for (int j = 0; j < cycles; j++) {
		for (int i = 0; i < total; i++) {
			double vy = axis_y.at(i);
			double vx = axis_x.at(i);
			short sample[2];
			sample[0] = (short) (vy / 100 * 32767);
			sample[1] = (short) (vx / 100 * 32767);
			file.write((char *)sample, 4);
			numSamples ++;
			dataSize += 4;

			prog++;
			int p = prog * 100 / pTotal;
			if (ui.progress->value() != p) {
				ui.progress->setValue(p);
			}
		}
	}
	file.seekp(0);
	header.subChunk2Size = numSamples * header.numChannels * header.bitsPerSample/8;
	header.chunkSize = 28 + 8 +  dataSize;
	file.write ((char *)&header, sizeof(TWaveHeader));
	file.close();
	ui.progress->setValue(0);
	ui.progress->hide();

	if (ui.actionAutoPlay->isChecked()) {
		delete wave;
		wave = new QSound(wavefile, this);
		wave->setLoops(QSound::Infinite);
		wave->play();		
		//wave.setLoops(QSound::Infinite);		
	}	
}


void MainWindow::OnAutoPlayClick()
{
	if (ui.actionAutoPlay->isChecked() == false) {
		wave->stop();
	}
}


void MainWindow::OnAboutClick()
{
	AboutWin * about  = new AboutWin(this);
	about->setParent(this);
	about->setModal(true);
	about->exec();
	delete about;
}