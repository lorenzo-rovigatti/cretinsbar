/*
 * MainWindow.cpp
 *
 *  Created on: 04 feb 2017
 *      Author: lorenzo
 */

#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "../SoundUtils/SoundUtils.h"
#include "../Utilities/WavAnalyser.h"

#include <QFile>
#include <QAudioDeviceInfo>
#include <QAudioDecoder>
#include <QAudioOutput>

namespace cb {

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), _ui(new Ui::MainWindow) {
	_ui->setupUi(this);
}

MainWindow::~MainWindow() {

}

void MainWindow::play() {
    QFile input("/home/lorenzo/nothing.wav");
    input.open(QIODevice::ReadOnly);
    QByteArray audio_data = input.readAll();
    QAudioFormat audio_format = WavAnalyser::format(audio_data);
    audio_format.setCodec("audio/pcm");
	audio_format.setSampleType(QAudioFormat::Float);

    input.seek(0);
    QAudioOutput output(audio_format);
    output.setVolume(1.0);
    output.start(&input);

	QEventLoop loop;
	QObject::connect(&output, SIGNAL(stateChanged(QAudio::State)), &loop, SLOT(quit()));
	do {
		loop.exec();
	} while(output.state() == QAudio::ActiveState);
}

} /* namespace cb */
