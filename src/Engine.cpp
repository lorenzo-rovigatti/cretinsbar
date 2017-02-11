/*
 * Engine.cpp
 *
 *  Created on: 05 feb 2017
 *      Author: lorenzo
 */

#include "Engine.h"

#include "SoundUtils/SoundUtils.h"

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
	 SoundUtils::Instance()->setup(_audio_format.sampleRate(), _audio_format.channelCount(), 30., _audio_format.sampleRate());
	 _out_file = SoundUtils::Instance()->process(*_wav_file);

	 qDebug() << _wav_file->data()->size() << _out_file->data()->size();

	_audio_output_IO_device.close();
	_audio_output_IO_device.setBuffer(_out_file->data());
	_audio_output_IO_device.open(QIODevice::ReadOnly);
	_audio_output->start(&_audio_output_IO_device);
}

void Engine::initialise() {
	_wav_file = std::unique_ptr<WavInFile>(new WavInFile("/home/lorenzo/nothing.wav"));
	_audio_format = _wav_file->format();
	_audio_output = new QAudioOutput(_audio_output_device, _audio_format, this);
	_audio_output->setVolume(0.5);
}

void Engine::reset() {
	delete _audio_output;
}

} /* namespace cb */
