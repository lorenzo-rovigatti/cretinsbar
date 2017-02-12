/*
 * Engine.h
 *
 *  Created on: 05 feb 2017
 *      Author: lorenzo
 */

#ifndef SRC_ENGINE_H_
#define SRC_ENGINE_H_

#include <memory>

#include <QObject>
#include <QBuffer>
#include <QByteArray>
#include <QAudioDeviceInfo>
#include <QAudioFormat>
#include "SoundUtils/Wave.h"

class QAudioOutput;

namespace cb {

class Engine: public QObject {
	Q_OBJECT;

public:
	Engine(QObject *parent);
	virtual ~Engine();

	void initialise();

public slots:
	void start_playback();

private slots:
	void _handle_state_changed(QAudio::State newState);
    void _audio_notify();

signals:
	/**
     * Format of audio data has changed
     */
	void format_changed(const QAudioFormat &new_format);

	/**
	 * Buffer containing audio data has changed.
	 * \param position Position of start of buffer in bytes
	 * \param buffer   Buffer
	 */
	void buffer_changed(qint64 position, qint64 length, const QByteArray &buffer);

	 /**
	 * Position of the audio output device has changed.
	 * \param position Position in bytes
	 */
	void play_position_changed(qint64 position);

private:
	void reset();
	void _set_play_position(qint64 position);

private:
	QAudioDeviceInfo _audio_output_device;
	QAudioOutput *_audio_output;
	QAudioFormat _audio_format;
    QBuffer _audio_output_IO_device;
    std::unique_ptr<Wave> _wav_file, _out_file;
    QByteArray _data;
    qint64 _play_position;
};

} /* namespace cb */

#endif /* SRC_ENGINE_H_ */
