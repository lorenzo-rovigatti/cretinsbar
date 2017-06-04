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
#include <QFileInfo>

#include <mpg123.h>

namespace cb {

Engine::Engine(QObject *parent) :
				QObject(parent),
				_audio_output_device(QAudioDeviceInfo::defaultOutputDevice()),
				_audio_output(nullptr),
				_start_from_time(0),
				_end_at_time(-1),
				_play_time(0),
				_volume(1.0),
				_curr_tempo_change(0.0),
				_curr_pitch_change(0) {

	mpg123_init();
}

Engine::~Engine() {
	mpg123_exit();
}

void Engine::_load_wave(const QString &filename) {
	_wav_file = std::unique_ptr<Wave>(new Wave(filename));
}

void Engine::_load_mp3(const QString &filename) {
	int m_err;
	mpg123_handle *mh = mpg123_new(NULL, &m_err);
	size_t buffer_size = mpg123_outblock(mh);
	int m_res = mpg123_open(mh, filename.toStdString().c_str());
	if(m_res == MPG123_OK) {
		int channels, encoding;
		long rate;
		mpg123_getformat(mh, &rate, &channels, &encoding);

		// encsize returns the size in bytes
		int bits = mpg123_encsize(encoding)*8;
		_wav_file = std::unique_ptr<Wave>(new Wave(channels, rate, bits));

		size_t done;
		unsigned char *buffer = new unsigned char[buffer_size];
		while (mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK) {
			_wav_file->append_samples((char *) buffer, buffer_size);
		}

		mpg123_close(mh);
		mpg123_delete(mh);
	}
}

void Engine::load(const QString &filename) {
	_reset();

	QFileInfo file_info(filename);
	QString extension = file_info.completeSuffix();
	if(extension == "wav") _load_wave(filename);
	else if(extension == "mp3") _load_mp3(filename);
	else {
		QString error = QString("Unsupported file extension '%1'").arg(extension);
		throw std::runtime_error(error.toStdString());
	}

	_audio_format = _wav_file->format();

	_out_file = std::unique_ptr<Wave>(new Wave(*_wav_file));

	_audio_output_IO_device.setBuffer(_out_file->data());
	_audio_output_IO_device.open(QIODevice::ReadOnly);

	_audio_output = new QAudioOutput(_audio_output_device, _audio_format, this);
	_audio_output->setNotifyInterval(10);
	connect(_audio_output, &QAudioOutput::stateChanged, this, &Engine::_handle_state_changed);
	connect(_audio_output, &QAudioOutput::notify, this, &Engine::_audio_notify);

	set_boundaries(0, -1);
}

const QByteArray *Engine::data() {
	return _out_file->data();
}

void Engine::set_boundaries(qint64 start_us, qint64 end_us) {
	if(is_ready()) {
		stop();

		_seek_buffer(start_us);
		_start_from_time = start_us;
		_end_at_time = (end_us > 0) ? end_us : duration()*1000000;
		emit play_position_changed(_start_from_time);
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

qreal Engine::duration() {
	if(is_ready()) return _wav_file->duration();
	else return 0.;
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
	_start_from_time = 0;
	_end_at_time = -1;
	_set_play_time(0);
}

void Engine::play(qreal tempo_change, int pitch_change) {
	if(is_ready() && _audio_output->state() != QAudio::ActiveState) {
		if(tempo_change != _curr_tempo_change || pitch_change != _curr_pitch_change) {
			_curr_tempo_change = tempo_change;
			_curr_pitch_change = pitch_change;
			_process(tempo_change, pitch_change);
			_seek_buffer(_start_from_time);
		}

		if(_audio_output_IO_device.atEnd()) _seek_buffer(_start_from_time);
		if(_audio_output->state() == QAudio::SuspendedState) _audio_output->resume();
		else _audio_output->start(&_audio_output_IO_device);
	}
}

void Engine::pause() {
	if(is_ready() && _audio_output->state() == QAudio::ActiveState) {
		_audio_output->suspend();
	}
}

void Engine::stop() {
	if(is_ready()) {
		_audio_output->stop();
		_seek_buffer(_start_from_time);
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
			if(!_audio_output_IO_device.atEnd()) _audio_output->start(&_audio_output_IO_device);
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
	qint64 elapsed_time = _from_real_to_original_time(_audio_output->processedUSecs());
	_set_play_time(elapsed_time);
}

void Engine::_set_play_time(qint64 elapsed_time) {
	if(elapsed_time < 0) elapsed_time = 0;
	qint64 new_time = _start_from_time + elapsed_time;
	if(_end_at_time >= 0 && new_time >= _end_at_time) {
		stop();
		emit ended();
	}
	else {
		const bool changed = (_play_time != new_time);
		_play_time = _start_from_time + elapsed_time;
		if(changed) emit play_position_changed(_play_time);
	}
}

void Engine::_process(qreal tempo_change, int pitch_change) {
	_out_file = SoundUtils::Instance()->process(*_wav_file, tempo_change, pitch_change);

	_audio_output_IO_device.close();
	_audio_output_IO_device.setBuffer(_out_file->data());
	_audio_output_IO_device.open(QIODevice::ReadOnly);
}

} /* namespace cb */
