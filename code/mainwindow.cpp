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

	connect(ui.sbZoom, SIGNAL(sliderReleased()), this, SLOT(OnRefresh()));
	connect(ui.edFileLength, SIGNAL(editingFinished()), this, SLOT(OnRefresh()));
	connect(ui.edPeriod, SIGNAL(editingFinished()), this, SLOT(OnRefresh()));
	connect(ui.cbUseOnlyXComponents, SIGNAL(stateChanged(int)), this, SLOT(OncbUseOnlyXComponents(int)));	
	

	sampleRate = 48000;
	duration = 0.02;
	fileLength = 1;

	ui.edPeriod->setText("0.02");
	ui.edFileLength->setText("10");	


	wave = new QSound("", this);
	wave->setLoops(QSound::Infinite);

	fileMonitor = new QFileSystemWatcher (this);
	connect(fileMonitor, SIGNAL(fileChanged(QString)), this, SLOT(OnFileChanged(QString)));

	

	zoom = double(ui.sbZoom->value()) / 100.0;

	ui.progress->setValue(0);
	ui.progress->show();	
}

MainWindow::~MainWindow()
{
	
}

void MainWindow::OnLoadFile()	
{
	QString newfilename = QFileDialog::getOpenFileName(this,	tr("Open Image"), "", tr("SVG Image Files (*.svg)"));
	
	if (QFileInfo(newfilename).exists() &&  QFileInfo(newfilename).isFile()) {	
		bool b = fileMonitor->removePath(filename);
		filename = newfilename;
		b = fileMonitor->addPath(filename);
		OnRefresh();	
	}
}

void MainWindow::DecodeLine(QString line, bool relative)
{
	points.clear();
	int p = line.indexOf(" ");
	bool first = true;
	TVecPoint last = {0, 0};
	while (line.size() > 0) {
		//QString mline = line.SubString(1, p-1);
		QString mline = line.left(p);
		//mline = mline.right(mline.size()-1);
		//line = line.Delete(1, mline.Length() + 1);
		line = line.remove(0, mline.size() + 1);

		int pv = mline.indexOf(",");

		if (pv > 0) {
			QString xs = mline.left(pv-1);
			//QString ys = mline.SubString(pv+1, mline.Length());
			QString ys = mline.right(mline.size()-pv-1);

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

void MainWindow::DecodeLine2(QString line, bool relative, TVecPoint & lastPoint)
{
	//points.clear();
	//line.remove(0,1);
	bool isCurve = false;
	/*if (line == "M471.96,628.1c0,14.09-11.423,25.512-25.513,25.512h-317.71   c-14.09,0-25.512-11.422-25.512-25.512V160.835c0-14.09,11.422-25.512,25.512-25.512h317.71c14.09,0,25.513,11.422,25.513,25.512   V628.1z") {
		isCurve = true;
	}*;*/
	
	if (line.contains("-8.7e-4")) {
		isCurve = true;
	}
	int p = line.indexOf(" ");
	bool first = true;
	//TVecPoint last = {0, 0};
	QStringList list;
	while (line.size() > 0) {
		int p = line.indexOf(QRegExp("[a-dA-Df-z:F-Z\\s:]"), 1);//[a-zA-Z\\s:]"),1);
		if (p >= 0) {
			QString mline = line;
			mline.remove(p, mline.size());
			list.append(mline);
			line.remove(0,p);
		} else {
			list.append(line);
			break;
		}
	}

	//QString xx = list[199];
	//QString xx1 = list[200];
	//str = "Some  text\n\twith  strange whitespace.";
	//list = line.split(QRegExp("[a-zA-Z\\s:]"));
	for (int i = 0; i < list.size(); i++) {
		//TVecPoint point = DecodePoint(mline);
		QString mline = list[i].trimmed();
		int k=1; 
		while (k < mline.size()) {
			if (mline.at(k)== '-') {				
				if (mline.at(k-1) != ',') {
					mline.insert(k, ",");
					k++;
				}
			}
			k++;
		}
		if (mline == "") {
			continue;
		}

		QString type = mline.left(1);
		QString utype = mline.left(1).toUpper();		
		if ((type.at(0)=='-') || ((type.at(0) >= '0') && (type.at(0) <= '9'))) {
			if (relative) {
				type = "l";
			} else {
				type = "L";
			}
			utype = "L";
		} else {
			mline = mline.right(mline.size()-1);
		}
		relative = (utype != type);

		if (mline.trimmed()== "") {
			if (i < list.size()-1) {
				list[i+1] = type + list[i+1].trimmed();
				continue;
			}
		}
		/*if (mline == "-14.09,0-25.512-11.422-25.512-25.512") {
			relative = true;
		}*/
		if (utype == "Z") {			
			points.push_back(points.at(0));
		} else {
			//QStringList mlist1 = mline.split(QRegExp(","),QString::SkipEmptyParts);
			QStringList mlist = mline.split(",",QString::SkipEmptyParts);
			/*while (mline.size() > 0) {
				int p1 = mline.indexOf(",");
				int p2 = mline.indexOf("-",1);
				if (p1 >= 0) {
					if (p1 > p2) {
						if (p2 >= 0) {
							p1 = p2;
						}
					}
				} else {
					p1 = p2;
				}
				if (p1 > 0) {
					QString numb = mline.left(p1);
					mline.remove(0, p1+1);
					mlist.append(numb);
				} else {
					mlist.append(mline);
					mline = "";					
				}				
			}*/
			/*for (int j = 0; j < mlist1.size(); j++) {
				QString numb = mlist1[j];
				int pp = numb.indexOf("-");
				while (pp > 0) {
					QString numb1 = numb.left(pp);
					numb = numb.right(numb.size() - pp);
					mlist.append(numb1);
					pp = numb.indexOf("-");
					//mlist.append(numb2);
				} //else {
				if (numb.size() > 0) { 
					mlist.append(numb);
				}
			}*/
			TVecPoint point;
			QString xs = "";
			QString ys = "";
			if ((utype == " ") || (utype == "L")) {
				xs = mlist[0];
				ys = mlist[1];
				//point.x = xs.toDouble();
				//point.y = ys.toDouble();
			}
			if (utype == "M") {
				xs = mlist[0];
				ys = mlist[1];
				//point.x = xs.toDouble();
				//point.y = ys.toDouble();
			}
			if (utype == "C") {
				if (mlist.size() < 6) {
					xs = "";
				} else {
					xs = mlist[4];
					ys = mlist[5];
				}
				//point.x = xs.toDouble();
				//point.y = ys.toDouble();
			}
			if (utype == "H") {
				xs = mlist[0];
				ys = QString::number(lastPoint.y);			
			}
			if (utype == "V") {
				ys = mlist[0];
				xs = QString::number(lastPoint.x);			
			}
		
			try {
				point.x = xs.toDouble();
				point.y = ys.toDouble();
			} catch (...) {
				point.x = 0;
				point.y = 0;
			}

			if (relative) {
				//point.x += lastPoint.x;
				//point.y += lastPoint.y;
			}
			memcpy(&lastPoint, &point, sizeof(TVecPoint));

			points.push_back(point);
		}	
		
	}
}

/*
		
		

	//while (line.size() > 0) {
		//QString mline = line.SubString(1, p-1);
		//]//int p = 
		//QString mline = line.left(p);
		line = line.remove(0, mline.size() + 1);

		QString xs ="";
		QString ys ="";
		int pv = mline.indexOf(",");
		if (isCurve) {
		} else {			
			int pe = mline.indexOf(" ");
			if (pv >= 0) {
				if (pe < pv) {
					pe = mline.size();
				}
				xs = mline.left(pv);
				ys = mline.left(pe);
				ys = ys.right(ys.size() - pv - 1);		
			}
		}
	
			//mline.remove(0, p
		if (pv >= 0) {

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
				relative = false;
			}

			if (mline == "c") {
				isCurve = true;
				relative = true;
			}

			if (mline == "L") {
				isCurve = true;
				relative = false;
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


*/
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
	double ratio = h / w;

	for (unsigned int i = 0; i < points.size(); i++) {
		double x = points.at(i).x;
		double y = points.at(i).y;
		//x = (x - minx) * 80 / w - 50 + 10;
		//y = ((y - miny) * 80 / h - 50 + 10) * ratio;

		x = (x - minx)  / w - 0.5;
		y = ((y - miny) / h ) * ratio - 0.5;

		/*if (i > 0) {
			x = x * (1 + (i * xCompensate));
			y = y * (1 + (i * yCompensate));
		}*/

		if (i > 0) {
			double x1 = points.at(i-1).x;
			double y1 = points.at(i-1).y;
			if (abs(x-x1) < w/200) {
				//x = x * xCompensate;
			}
			if (abs(y-y1) < h/200) {
				//y = y * yCompensate;
			}
		}
		points.at(i).x = x;
		points.at(i).y = y;
	}
}



double MainWindow::distance(TVecPoint p1, TVecPoint p2)
{
	double deltax = abs(p1.x - p2.x);
	double deltay = abs(p1.y - p2.y);

	double dist = sqrt(deltax * deltax + deltay * deltay);
	return dist;
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
		if (ui.cbUseOnlyXComponents-> isChecked() == false) {
			delta = distance(points.at(i), points.at(i+1));
		}
		totalx += delta;
	}

	// "times" hold all "x" values converted to real-time
	vector <double> times;
	times.push_back(0.0);
	for (int i = 0; i < psize; i++) {
		double delta;
		if (points.at(i).x > points.at(i+1).x) {
			delta = abs(points.at(i).x - points.at(i+1).x);
		} else {
			delta = points.at(i+1).x - points.at(i).x;
		}
		if (ui.cbUseOnlyXComponents-> isChecked() == false) {
			delta = distance(points.at(i), points.at(i+1));
		}
		double time = (delta/totalx) * duration;
		time = time + times.at(i);
		times.push_back(time);
	}

	unsigned int numberOfPoints = sampleRate * duration;
	int idx = 0;
	double timeStep = double(1.0) / sampleRate;
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
	vector <int>pp;
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
		//x = x + i * 0.01;
		//double x = points.at(idx + 1).x + grad * (points.at(idx + 1).x - points.at(i).x);

		axis_x.push_back(x);
		//axis_y.push_back(-(y));
		axis_y.push_back(-y);
		
		time = time + timeStep;
		if (time >= lim_sup) {
			pp.push_back(i);			
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

	/*QTextStream in(&file);

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

	points.clear();
	TVecPoint lastPoint = {0, 0};

	while (line.size() > 0) {
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

		line.remove(0, 3);

		pos1 = line.indexOf("\"");
		if (pos1 < 0) {
			QMessageBox::information(0, "error", "Could not find end of path on SVG file");
			return;
		}

		QString subline = line;
		subline.remove(pos1, line.size());
		line.remove(0, pos1);		
		DecodeLine(subline, relative, lastPoint);
		pos1 = line.indexOf("<path");
		if (pos1 >= 0) {
			line.remove(0, pos1 + 5);
		} else {
			break;
		}
	}

	*/
	QStringList list;
	QTextStream in(&file);	
	while(!in.atEnd()) {
		QString line = in.readLine();
		list.append(line);
	}
	file.close();

	QString line;
	do {
		line = list[0];
		list.removeAt(0);
	} while ((list.size() > 0) && (line.indexOf("<path") < 0));

	if (list.size() < 1) {
		QMessageBox::information(0, "error", "Could not find start of path on SVG file");
		return;
	}

	line = list[0];
	line = line.toLower();
	bool relative = true;
	int p = line.indexOf("d=\"m ");
	if (p < 0) {
		p = line.indexOf("d=\"M ");
		relative = false;
	}
	while (p < 0)  {
		list.removeAt(0);
		line = list[0];
		p = line.indexOf("d=\"m ");
		relative = true;
		if (p < 0) {
			p = line.indexOf("d=\"M ");
			relative = false;
		}
	}

	if (p >= 0) {
		line = line.remove(0, p + 4);
		while (line.at(0) == " ") {
			line = line.right(line.size()-1);
		}
	}

	if (line.indexOf("\"") < 0) {
		QMessageBox::information(0, "error", "Could not find end of path on SVG file");
		return;
	}

	line = line.remove(line.indexOf("\""), line.size());

	DecodeLine(line, relative);

	duration = ui.edPeriod->text().toDouble();
	if ((duration < 0.0001) || (duration > 100)) {
		duration = 0.02;
		ui.edPeriod->setText("0.02");		
	}
	//QString t = ui.edFileLength->text();
	fileLength = ui.edFileLength->text().toDouble();
	if ((fileLength < 0.0001) || (fileLength > 10000)) {
		fileLength = 10;
		ui.edFileLength->setText("10");	
	}

	zoom = double(ui.sbZoom->value()) / 100.0;
	
	xCompensate = ui.sbCompensateX->value();
	yCompensate = ui.sbCompensateY->value();

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

	//double centerX = 0.5; // - double(ui.centerX->value()) / 100.0;
	//double centerY = 0.5 - double(ui.centerY->value()) / 100.0;
	for (int j = 0; j < cycles; j++) {
		for (int i = 0; i < total; i++) {
			double vy = axis_y.at(i);
			double vx = axis_x.at(i);
			short sample[2];
			//sample[0] = (short) (zoom * vy / 50.0 * 32767.0);
			//sample[1] = (short) (zoom * vx / 50.0 * 32767.0  + 0 * 32767.0);

			sample[0] = (short) (zoom * (vy) * 32767.0);
			sample[1] = (short) (zoom * (vx) * (32767.0));
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

void MainWindow::OncbUseOnlyXComponents(int state)
{
	OnRefresh();
}