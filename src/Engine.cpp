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
	, _audio_output(nullptr)
	, _play_position(0) {
}

Engine::~Engine() {

}

void Engine::initialise() {
	_play_position = 0;

	_wav_file = std::unique_ptr<Wave>(new Wave("/home/lorenzo/nothing2.wav"));
	_audio_format = _wav_file->format();
	emit format_changed(&_audio_format);

	_out_file = SoundUtils::Instance()->process(*_wav_file, 100, 0);
	emit buffer_changed(0, _out_file->data()->size(), *_out_file->data());

	_audio_output_IO_device.setBuffer(_out_file->data());
	_audio_output_IO_device.open(QIODevice::ReadOnly);

	_audio_output = new QAudioOutput(_audio_output_device, _audio_format, this);
	_audio_output->setNotifyInterval(50);
	connect(_audio_output, &QAudioOutput::stateChanged, this, &Engine::_handle_state_changed);
	connect(_audio_output, &QAudioOutput::notify, this, &Engine::_audio_notify);

}

void Engine::reset() {
	delete _audio_output;
}

void Engine::start_playback() {
	_audio_output->start(&_audio_output_IO_device);
}

void Engine::_handle_state_changed(QAudio::State newState) {
	switch(newState) {
	case QAudio::IdleState:
		// Finished playing (no more data)
		_audio_output->stop();
		_audio_output_IO_device.close();
		delete _audio_output;
		break;

	case QAudio::StoppedState:
		// Stopped for other reasons
		if(_audio_output->error() != QAudio::NoError) {
			// Error handling
		}
		break;

	default:
		// ... other cases as appropriate
		break;
	}
}

void Engine::_audio_notify() {
	const qint64 play_pos = _audio_output->processedUSecs();
	_set_play_position(play_pos);
}

void Engine::_set_play_position(qint64 position) {
	const bool changed = (_play_position != position);
	_play_position = position;
	if(changed) emit play_position_changed(position);
}

} /* namespace cb */
