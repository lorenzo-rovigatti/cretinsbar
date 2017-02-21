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

Engine::Engine(QObject *parent) :
				QObject(parent),
				_audio_output_device(QAudioDeviceInfo::defaultOutputDevice()),
				_audio_output(nullptr),
				_base_pos(0),
				_play_pos(0),
				_volume(1.0),
				_curr_tempo_change(0.0),
				_curr_pitch_change(0) {
}

Engine::~Engine() {

}

void Engine::load(const QString &filename) {
	reset();

	_wav_file = std::unique_ptr<Wave>(new Wave(filename));
	_audio_format = _wav_file->format();
	emit format_changed(&_audio_format);

	_out_file = std::unique_ptr<Wave>(new Wave(*_wav_file));
	emit buffer_changed(0, _out_file->data()->size(), *_out_file->data());

	_audio_output_IO_device.setBuffer(_out_file->data());
	_audio_output_IO_device.open(QIODevice::ReadOnly);

	_audio_output = new QAudioOutput(_audio_output_device, _audio_format, this);
	_audio_output->setNotifyInterval(10);
	_audio_output->setBufferSize(_out_file->data()->size());
	connect(_audio_output, &QAudioOutput::stateChanged, this, &Engine::_handle_state_changed);
	connect(_audio_output, &QAudioOutput::notify, this, &Engine::_audio_notify);
}

void Engine::jump_to(qint64 us) {
	if(is_ready()) {
		stop();

		qint64 n_byte = _out_file->bytes_from_us(us);
		_audio_output_IO_device.seek(n_byte);
		_base_pos = us;
		emit play_position_changed(_base_pos);
	}
}

void Engine::set_volume(qreal new_volume) {
	if(is_ready() && new_volume > 0. && new_volume <= 1.0) _audio_output->setVolume(new_volume);
}

bool Engine::is_playing() {
	return _audio_output->state() == QAudio::ActiveState;
}

bool Engine::is_ready() {
	return _audio_output != nullptr;
}

void Engine::reset() {
	if(is_ready()) {
		_audio_output->stop();
		_audio_output_IO_device.close();
		delete _audio_output;
		_audio_output = nullptr;
	}
	_base_pos = 0;
	_set_play_position(0);
}

void Engine::play(qreal tempo_change, int pitch_change) {
	if(is_ready() && _audio_output->state() != QAudio::ActiveState) {
		if(tempo_change != _curr_tempo_change || pitch_change != _curr_pitch_change) {
			_curr_tempo_change = tempo_change;
			_curr_pitch_change = pitch_change;
			_process(tempo_change, pitch_change);
		}

		if(_audio_output_IO_device.atEnd()) _audio_output_IO_device.seek(0);
		_audio_output->start(&_audio_output_IO_device);
	}
}

void Engine::pause() {
	if(is_ready() && _audio_output->state() == QAudio::ActiveState) {
		_audio_output->suspend();
		_base_pos = _play_pos;
	}
}

void Engine::stop() {
	if(is_ready()) {
		_audio_output->stop();
		_audio_output_IO_device.seek(0);
		_base_pos = 0;
		_set_play_position(0);
	}
}

void Engine::_handle_state_changed(QAudio::State newState) {
	switch(newState) {
	case QAudio::StoppedState:
		emit stopped();
		// Stopped for other reasons
		if(_audio_output->error() != QAudio::NoError) {
			qDebug() << _audio_output->error();
		}
		_set_play_position(0);
		break;
	case QAudio::IdleState:
		emit stopped();
		_set_play_position(0);
		break;
	case QAudio::SuspendedState:
		emit paused();
		break;
	case QAudio::ActiveState:
		emit playing();
		break;
	default:
		break;
	}
}

void Engine::_audio_notify() {
	_set_play_position(_audio_output->processedUSecs());
}

void Engine::_set_play_position(qint64 position) {
	qint64 new_pos = _base_pos + position;
	const bool changed = (_play_pos != new_pos);
	_play_pos = _base_pos + position;
	if(changed) emit play_position_changed(_play_pos);
}

void Engine::_process(qreal tempo_change, int pitch_change) {
	_out_file = SoundUtils::Instance()->process(*_wav_file, tempo_change, pitch_change);
	emit buffer_changed(0, _out_file->data()->size(), *_out_file->data());

	_audio_output_IO_device.close();
	_audio_output_IO_device.setBuffer(_out_file->data());
	_audio_output_IO_device.open(QIODevice::ReadOnly);
}

} /* namespace cb */
