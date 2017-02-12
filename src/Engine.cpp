/*
 * Engine.cpp
 *
 *  Created on: 05 feb 2017
 *      Author: lorenzo
 */

#include "Engine.h"

#include "SoundUtils/SoundUtils.h"
#include "SoundUtils/Wave.h"

#include <QAudioOutput>
#include <QFile>
#include <QAudioFormat>

namespace cb {

Engine::Engine(QObject *parent)
	: QObject(parent)
	, _audio_output_device(QAudioDeviceInfo::defaultOutputDevice())
	, _audio_output(nullptr) {

	initialise();
}

Engine::~Engine() {
}

void Engine::start_playback() {
	_out_file = SoundUtils::Instance()->process(*_wav_file, -20, 0);

	_audio_output_IO_device.setBuffer(_out_file->data());
	_audio_output_IO_device.open(QIODevice::ReadOnly);
	_audio_output->start(&_audio_output_IO_device);
}

void Engine::initialise() {
	_wav_file = std::unique_ptr<Wave>(new Wave("/home/lorenzo/nothing2.wav"));
	_audio_format = _wav_file->format();

	_audio_output = new QAudioOutput(_audio_output_device, _audio_format, this);
}

void Engine::reset() {
	delete _audio_output;
}

} /* namespace cb */
