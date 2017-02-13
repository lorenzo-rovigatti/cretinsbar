/*
 * MainWindow.cpp
 *
 *  Created on: 04 feb 2017
 *      Author: lorenzo
 */

#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "../Engine.h"
#include "../SoundUtils/SoundUtils.h"
#include "qcustomplot/qcustomplot.h"

#include <QAudioFormat>

namespace cb {

MainWindow::MainWindow(Engine *engine, QWidget *parent)
	: QMainWindow(parent)
	, _engine(engine)
	, _ui(new Ui::MainWindow) {
	_ui->setupUi(this);
	_init_plot();

	connect(_engine, &Engine::format_changed, this, &MainWindow::format_changed);
	connect(_engine, &Engine::buffer_changed, this, &MainWindow::buffer_changed);
	connect(_engine, &Engine::play_position_changed, this, &MainWindow::play_position_changed);
}

MainWindow::~MainWindow() {

}

void MainWindow::format_changed(const QAudioFormat *new_format) {
	long max_val = 2 << (new_format->sampleSize() - 2);
	long min_val = (new_format->sampleType() == QAudioFormat::UnSignedInt) ? 0: -max_val;
	_plot->yAxis->setRange(min_val, max_val);
	_audio_format = new_format;
}

void MainWindow::buffer_changed(qint64 position, qint64 length, const QByteArray &buffer) {
	_plot->clearGraphs();
	int bytes = _audio_format->sampleSize()/8;
	long n_samples = length/bytes;
	double length_in_seconds = n_samples/(double)_audio_format->sampleRate()/_audio_format->channelCount();
	long increment = _audio_format->channelCount();

	qDebug() << length << n_samples << increment << length_in_seconds;

	const short *samples = reinterpret_cast<const short *>(buffer.data());
	QVector<double> x_data(n_samples);
	QVector<double> y_data(n_samples);
	for(int i = 0; i < n_samples; i += increment) {
		int idx = i/increment;
		x_data[idx] = idx/(qreal)_audio_format->sampleRate();
		y_data[idx] = (double) samples[i];
	}

	QCPGraph *graph = _plot->addGraph();
	graph->setPen(QPen(QColor("black")));
	graph->setData(x_data, y_data);
	_plot->xAxis->setRange(0, length_in_seconds);
}

void MainWindow::play_position_changed(qint64 position) {
	double pos_in_sec = position/(double)1000000.;
	_position->point1->setCoords(pos_in_sec, -1);
	_position->point2->setCoords(pos_in_sec, 1);
	_plot->replot();
	qDebug() << "dio" << position << pos_in_sec;
}

void MainWindow::on_mouse_move(QMouseEvent *event) {
	// transform the mouse position to x,y coordinates and show them in the status bar
	double x = _plot->xAxis->pixelToCoord(event->pos().x());
	QString msg = QString("%1 s").arg(x, 0, 'f', 2);
	_ui->statusbar->showMessage(msg);
}

void MainWindow::_init_plot() {
	_plot = _ui->plot;
	_plot->setMaximumHeight(200);
	_plot->setInteractions(QCP::iRangeZoom | QCP::iRangeDrag);
	_plot->axisRect()->setRangeDrag(Qt::Horizontal);
	_plot->axisRect()->setRangeZoom(Qt::Horizontal);

//	_plot->xAxis->setTicks(false);
	_plot->xAxis->setTickLength(0, 0);
	_plot->xAxis->setSubTicks(false);
	_plot->xAxis->setTickLabels(true);
	_plot->xAxis->setBasePen(Qt::NoPen);

	_plot->yAxis->setTickLabels(false);
	_plot->yAxis->setTicks(false);
	_plot->yAxis->grid()->setVisible(false);
	_plot->yAxis->setBasePen(Qt::NoPen);

	_position = new QCPItemStraightLine(_plot);
	_position->point1->setCoords(0, -1);
	_position->point2->setCoords(0, 1);
	QPen line_pen(QColor("red"));
	line_pen.setWidth(3);
	_position->setPen(line_pen);
	_plot->addLayer("play_position", 0, QCustomPlot::limAbove);
	_position->setLayer("play_position");

	connect(_plot, &QCustomPlot::mouseMove, this, &MainWindow::on_mouse_move);
}

} /* namespace cb */
