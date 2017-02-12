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

#include <QAudioFormat>

namespace cb {

MainWindow::MainWindow(Engine *engine, QWidget *parent)
	: QMainWindow(parent)
	, _engine(engine)
	, _ui(new Ui::MainWindow) {
	_ui->setupUi(this);

	connect(_engine, &Engine::format_changed, this, &MainWindow::format_changed);
	connect(_engine, &Engine::buffer_changed, _ui->waveform, &Waveform::buffer_changed);
	connect(_engine, &Engine::play_position_changed, _ui->waveform, &Waveform::audio_position_changed);
}

MainWindow::~MainWindow() {

}

void MainWindow::format_changed(const QAudioFormat &new_format) {
	_ui->waveform->initialize(new_format, 4096, 50000);
}

} /* namespace cb */
