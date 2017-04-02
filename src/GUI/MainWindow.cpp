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
#include "WaveForm.h"

#include <QAudioFormat>

namespace cb {

MainWindow::MainWindow(Engine *engine, QWidget *parent) :
				QMainWindow(parent),
				_pos_layer("play_position"),
				_engine(engine),
				_ui(new Ui::MainWindow) {
	_ui->setupUi(this);
	_init_plot();

	connect(_ui->action_open, &QAction::triggered, this, &MainWindow::_on_open);

	connect(_engine, &Engine::play_position_changed, _plot, &WaveForm::update_play_position);

	connect(_engine, &Engine::playing, this, &MainWindow::_engine_playing);
	connect(_engine, &Engine::paused, this, &MainWindow::_engine_paused);
	connect(_engine, &Engine::stopped, this, &MainWindow::_engine_stopped);
	connect(_engine, &Engine::ended, this, &MainWindow::_engine_at_end);

	connect(_ui->play_button, &QPushButton::toggled, this, &MainWindow::_toggle_play);
	connect(_ui->stop_button, &QPushButton::clicked, this, &MainWindow::_stop);

	// right-align all the elements in the leftmost column of the tempo/pitch change grid layout
	for(int i = 0; i < _ui->slider_layout->rowCount(); i++) {
		_ui->slider_layout->itemAtPosition(i, 0)->setAlignment(Qt::AlignRight);
	}
}

MainWindow::~MainWindow() {

}

void MainWindow::load_in_engine(QString filename) {
	this->setEnabled(false);
	_ui->plot->setEnabled(false);
	_ui->play_button->setEnabled(false);
	_ui->stop_button->setEnabled(false);
	_ui->loop_button->setEnabled(false);
	_ui->pitch_slider->setEnabled(false);
	_ui->tempo_slider->setEnabled(false);
	_ui->plot_scrollbar->setEnabled(false);

	const QByteArray *buffer =_engine->load(filename);
	qint64 length = buffer->length();
	_plot->clearGraphs();
	int bytes = _engine->sample_size() / 8;
	long n_samples = length / bytes;
	qreal length_in_seconds = _engine->duration();
	long increment = _engine->channel_count();

	const short *samples = reinterpret_cast<const short *>(buffer->data());
	QVector<qreal> x_data(n_samples);
	QVector<qreal> y_data(n_samples);
	for(int i = 0; i < n_samples; i += increment) {
		int idx = i / increment;
		x_data[idx] = idx / (qreal) _engine->sample_rate();
		y_data[idx] = (qreal) samples[i];
	}

	QCPGraph *graph = _plot->addGraph();
	graph->setPen(QPen(QColor("black")));
	graph->setData(x_data, y_data);
	_ui->plot_scrollbar->setRange(0, length_in_seconds);
	_plot->xAxis->setRange(0, length_in_seconds);

	long max_val = 2 << (_engine->sample_size() - 2);
	long min_val = (_engine->sample_type() == QAudioFormat::UnSignedInt) ? 0 : -max_val;
	_plot->yAxis->setRange(min_val, max_val);

	_plot->replot();

	_ui->play_button->setEnabled(true);
	_ui->stop_button->setEnabled(true);
	_ui->loop_button->setEnabled(true);
	_ui->pitch_slider->setEnabled(true);
	_ui->tempo_slider->setEnabled(true);
	_ui->plot_scrollbar->setEnabled(true);
	_plot->setEnabled(true);
	this->setEnabled(true);
}

void MainWindow::_on_open() {
	QString filename = QFileDialog::getOpenFileName(this, tr("Open file"), "", tr("WAV files(*.wav)"));
	if(filename.size() > 0) load_in_engine(filename);
}

void MainWindow::_toggle_play(bool s) {
	if(s) {
		qreal tempo_change = (qreal) _ui->tempo_slider->value() - 100.;
		int pitch_change = _ui->pitch_slider->value();
		_engine->play(tempo_change, pitch_change);
	}
	else _engine->pause();
}

void MainWindow::_stop() {
	_engine->stop();
}

void MainWindow::_plot_on_mouse_release(QMouseEvent *event) {
	pair_qreal pq = _plot->selection_boundaries();
	qint64 start_us = pq.first*1000000;
	qint64 end_us = pq.second*1000000;

	_engine->set_boundaries(start_us, end_us);
}

void MainWindow::_engine_playing() {
	_ui->play_button->setChecked(true);
	_ui->play_button->setText("Pause");
}

void MainWindow::_engine_paused() {
	_ui->play_button->setChecked(false);
	_ui->play_button->setText("Play");
}

void MainWindow::_engine_stopped() {
	_ui->play_button->setChecked(false);
	_ui->play_button->setText("Play");
}

void MainWindow::_engine_at_end() {
	if(_ui->loop_button->isChecked()) _ui->play_button->click();
}

void MainWindow::_init_plot() {
	_plot = _ui->plot;
	_plot->init(_ui->plot_scrollbar);

	connect(_plot, &QCustomPlot::mouseRelease, this, &MainWindow::_plot_on_mouse_release);
	connect(_plot, SIGNAL(status_update(QString)), _ui->statusbar, SLOT(showMessage(QString)));
}

} /* namespace cb */
