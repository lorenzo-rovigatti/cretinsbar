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
	 SoundUtils::Instance()->setup(_audio_format.sampleRate(), _audio_format.channelCount(), 0., _audio_format.sampleRate());
	 SoundUtils::Instance()->process(*_wav_file, &_audio_data);
	 qDebug() << _audio_data.size() << 4*_wav_file->numSamples() << _wav_file->length_in_ms();

	_audio_output_IO_device.close();
	_audio_output_IO_device.setBuffer(&_audio_data);
	_audio_output_IO_device.open(QIODevice::ReadOnly);
	_audio_output->start(&_audio_output_IO_device);
}

void Engine::initialise() {
//    QFile input("/home/lorenzo/nothing.wav");
//    input.open(QIODevice::ReadOnly);
//    _audio_data_original = input.readAll();
//    _audio_format = WavAnalyser::format(_audio_data_original);
//    _audio_format.setCodec("audio/pcm");
//	_audio_format.setSampleType(QAudioFormat::Float);
//
//	_audio_data = _audio_data_original;

	_wav_file = std::unique_ptr<WavInFile>(new WavInFile("/home/lorenzo/nothing.wav"));
	_audio_format = _wav_file->format();
	_audio_output = new QAudioOutput(_audio_output_device, _audio_format, this);
	_audio_output->setVolume(0.5);
}

void Engine::reset() {
	delete _audio_output;
}

} /* namespace cb */
