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
class QString;

namespace cb {

class Engine: public QObject {
	Q_OBJECT;

public:
	Engine(QObject *parent);
	virtual ~Engine();

	void load(const QString &filename);
	void jump_to(qint64 us);
	void set_volume(qreal new_volume);

	bool is_playing();
	bool is_ready();

public slots:
	void play(qreal tempo_change, int pitch_change);
	void pause();
	void stop();

private slots:
	void _handle_state_changed(QAudio::State newState);
    void _audio_notify();

signals:
	/**
     * Format of audio data has changed
     */
	void format_changed(const QAudioFormat *new_format);

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

	void playing();
	void paused();
	void stopped();

private:
	void reset();
	void _set_play_position(qint64 position);

	/** Process the audio stored in _wav_file and store it in _out_file.
	 *
	 * The new audio will be generated according to the values passed in as parameters.
	 *
	 * @param tempo_change Change in tempo (in percentage)
	 * @param pitch_change Change in pitch (in number of semitones)
	 */
	void _process(qreal tempo_change, int pitch_change);

private:
	QAudioDeviceInfo _audio_output_device;
	QAudioOutput *_audio_output;
	QAudioFormat _audio_format;
    QBuffer _audio_output_IO_device;
    std::unique_ptr<Wave> _wav_file, _out_file;
    QByteArray _data;
    qint64 _base_pos, _play_pos;
    qreal _volume;
    qreal _curr_tempo_change;
    int _curr_pitch_change;
};

} /* namespace cb */

#endif /* SRC_ENGINE_H_ */
