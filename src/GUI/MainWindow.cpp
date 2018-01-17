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

#include <QMessageBox>
#include <QStringList>
#include <QAudioFormat>

namespace cb {

MainWindow::MainWindow(Engine *engine, QWidget *parent) :
		QMainWindow(parent), _pos_layer("play_position"), _engine(engine), _ui(new Ui::MainWindow) {
	_ui->setupUi(this);
	_init_plot();

	connect(_ui->action_open, &QAction::triggered, this, &MainWindow::_on_open);
	connect(_ui->action_export_all, &QAction::triggered, this, &MainWindow::_export_all);
	connect(_ui->action_export_selection, &QAction::triggered, this, &MainWindow::_export_selection);

	connect(_engine, &Engine::play_position_changed, _plot, &WaveForm::update_play_position);

	connect(_engine, &Engine::playing, this, &MainWindow::_engine_playing);
	connect(_engine, &Engine::paused, this, &MainWindow::_engine_paused);
	connect(_engine, &Engine::stopped, this, &MainWindow::_engine_stopped);
	connect(_engine, &Engine::ended, this, &MainWindow::_engine_at_end);

	connect(_ui->tempo_slider, &QSlider::valueChanged, this, &MainWindow::_on_slider_change);
	connect(_ui->pitch_slider, &QSlider::valueChanged, this, &MainWindow::_on_slider_change);

	connect(_ui->play_button, &QPushButton::toggled, this, &MainWindow::_toggle_play);
	connect(_ui->stop_button, &QPushButton::clicked, this, &MainWindow::_stop);

	// right-align all the elements in the leftmost column of the tempo/pitch change grid layout
	for(int i = 0; i < _ui->slider_layout->rowCount(); i++) {
		_ui->slider_layout->itemAtPosition(i, 0)->setAlignment(Qt::AlignRight);
	}

	_set_controls_state(false);
}

MainWindow::~MainWindow() {

}

void MainWindow::load_in_engine(QString filename) {
	this->setEnabled(false);
	_reset_controls();

	try {
		_engine->load(filename);
		_plot->load_wave(_engine);
	}
	catch(std::exception &e) {
		_show_critical(tr("Loading failed"), QString(e.what()));
	}

	this->setEnabled(true);
	_set_controls_state(true);
}

QString MainWindow::_supported_files_filter() {
	QStringList list;
	list.append(tr("All files(*.*)"));
	list.append(tr("\nWAV files(*.wav)"));
#ifndef NOMP3
	list.append(tr("\nmp3 files(*.mp3)"));
#endif

	return list.join('\n');
}

void MainWindow::_on_open() {
	QString filename = QFileDialog::getOpenFileName(this, tr("Open file"), "", _supported_files_filter());
	if(filename.size() > 0) load_in_engine(filename);
}

void MainWindow::_export_all() {
	_export(ALL);
}

void MainWindow::_export_selection() {
	_export(SELECTION);
}

void MainWindow::_export(export_type t) {
	QString filename = QFileDialog::getSaveFileName(this, tr("Export as"), "", _supported_files_filter());
	if(filename.size() > 0) {
		try {
			switch(t) {
			case ALL:
				_engine->export_all(filename);
				break;
			case SELECTION:
				_engine->export_selection(filename);
				break;
			}
		}
		catch(std::exception &e) {
			_show_critical(tr("Loading failed"), QString(e.what()));
		}
	}
}

void MainWindow::_toggle_play(bool s) {
	if(s) {
		qreal tempo_change = (qreal) _ui->tempo_slider->value() - 100.;
		int pitch_change = _ui->pitch_slider->value();

		this->setEnabled(false);
		_engine->play(tempo_change, pitch_change);
		this->setEnabled(true);
	}
	else _engine->pause();
}

void MainWindow::_stop() {
	_engine->stop();
}

void MainWindow::_plot_on_mouse_release(QMouseEvent *event) {
	pair_qreal pq = _plot->selection_boundaries();
	qint64 start_us = pq.first * 1000000;
	qint64 end_us = pq.second * 1000000;

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

void MainWindow::_on_slider_change() {
	if(_engine->is_playing()) _toggle_play(false);
}

void MainWindow::_init_plot() {
	_plot = _ui->plot;
	_plot->init(_ui->plot_scrollbar);

	connect(_plot, &QCustomPlot::mouseRelease, this, &MainWindow::_plot_on_mouse_release);
	connect(_plot, SIGNAL(status_update(QString)), _ui->statusbar, SLOT(showMessage(QString)));
}

void MainWindow::_reset_controls() {
	_ui->tempo_slider->setSliderPosition(100);
	_ui->pitch_slider->setSliderPosition(0);
	_ui->loop_button->setChecked(true);
}

void MainWindow::_set_controls_state(bool state) {
	_ui->plot->setEnabled(state);
	_ui->play_button->setEnabled(state);
	_ui->stop_button->setEnabled(state);
	_ui->loop_button->setEnabled(state);
	_ui->pitch_slider->setEnabled(state);
	_ui->tempo_slider->setEnabled(state);
	_ui->plot_scrollbar->setEnabled(state);
	_ui->menu_export->setEnabled(state);
}

void MainWindow::_show_critical(const QString &title, const QString &msg) {
	QMessageBox::critical(this, title, msg);
}

} /* namespace cb */
