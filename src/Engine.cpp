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
				_base_time(0),
				_play_time(0),
				_volume(1.0),
				_curr_tempo_change(0.0),
				_curr_pitch_change(0) {
}

Engine::~Engine() {

}

const QByteArray *Engine::load(const QString &filename) {
	_reset();

	_wav_file = std::unique_ptr<Wave>(new Wave(filename));
	_audio_format = _wav_file->format();

	_out_file = std::unique_ptr<Wave>(new Wave(*_wav_file));

	_audio_output_IO_device.setBuffer(_out_file->data());
	_audio_output_IO_device.open(QIODevice::ReadOnly);

	_audio_output = new QAudioOutput(_audio_output_device, _audio_format, this);
	_audio_output->setNotifyInterval(10);
//	_audio_output->setBufferSize(_out_file->data()->size());
	connect(_audio_output, &QAudioOutput::stateChanged, this, &Engine::_handle_state_changed);
	connect(_audio_output, &QAudioOutput::notify, this, &Engine::_audio_notify);

	return _out_file->data();
}

void Engine::seek(qint64 us) {
	if(is_ready()) {
		stop();

		_seek_buffer(us);
		_base_time = us;
		emit play_position_changed(_base_time);
	}
}

void Engine::set_volume(qreal new_volume) {
	if(is_ready() && new_volume > 0. && new_volume <= 1.0) _audio_output->setVolume(new_volume);
}

int Engine::channel_count() {
	return _audio_format.channelCount();
}

int Engine::sample_size() {
	return _audio_format.sampleSize();
}

int Engine::sample_rate() {
	return _audio_format.sampleRate();
}

QAudioFormat::SampleType Engine::sample_type() {
	return _audio_format.sampleType();
}

bool Engine::is_playing() {
	return _audio_output->state() == QAudio::ActiveState;
}

bool Engine::is_ready() {
	return _audio_output != nullptr;
}

void Engine::_reset() {
	if(is_ready()) {
		_audio_output->stop();
		_audio_output_IO_device.close();
		delete _audio_output;
		_audio_output = nullptr;
	}
	_base_time = 0;
	_set_play_time(0);
}

void Engine::play(qreal tempo_change, int pitch_change) {
	if(is_ready() && _audio_output->state() != QAudio::ActiveState) {
		if(tempo_change != _curr_tempo_change || pitch_change != _curr_pitch_change) {
			_curr_tempo_change = tempo_change;
			_curr_pitch_change = pitch_change;
			_process(tempo_change, pitch_change);
			_seek_buffer(_base_time);
		}

		if(_audio_output_IO_device.atEnd()) _seek_buffer(_base_time);
		_audio_output->start(&_audio_output_IO_device);
	}
}

void Engine::pause() {
	if(is_ready() && _audio_output->state() == QAudio::ActiveState) {
		_audio_output->suspend();
		_base_time = _play_time;
	}
}

void Engine::stop() {
	if(is_ready()) {
		_audio_output->stop();
		_seek_buffer(_base_time);
		_set_play_time(0);
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
		_set_play_time(0);
		break;
	case QAudio::IdleState:
		emit stopped();
		_set_play_time(0);
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

void Engine::_seek_buffer(qint64 new_time) {
	qint64 real_time = _from_original_to_real_time(new_time);
	_audio_output_IO_device.seek(_out_file->bytes_from_us(real_time));
}

qint64 Engine::_from_original_to_real_time(qint64 pos) {
	qreal factor = 100./(_curr_tempo_change + 100.);
	return pos*factor;
}

qint64 Engine::_from_real_to_original_time(qint64 pos) {
	qreal factor = (_curr_tempo_change + 100.)/100.;
	return pos*factor;
}

void Engine::_audio_notify() {
	qint64 original_time = _from_real_to_original_time(_audio_output->processedUSecs());
	_set_play_time(original_time);
}

void Engine::_set_play_time(qint64 time) {
	qint64 new_time = _base_time + time;
	const bool changed = (_play_time != new_time);
	_play_time = _base_time + time;
	if(changed) emit play_position_changed(_play_time);
}

void Engine::_process(qreal tempo_change, int pitch_change) {
	_out_file = SoundUtils::Instance()->process(*_wav_file, tempo_change, pitch_change);

	qDebug() << _out_file->get_n_samples() << _wav_file->get_n_samples();
	qDebug() << _out_file->bytes_from_us(_base_time) << _wav_file->bytes_from_us(_base_time);

	_audio_output_IO_device.close();
	_audio_output_IO_device.setBuffer(_out_file->data());
	_audio_output_IO_device.open(QIODevice::ReadOnly);
}

} /* namespace cb */
