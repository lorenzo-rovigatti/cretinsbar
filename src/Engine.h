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

	const QByteArray *load(const QString &filename);
	void set_boundaries(qint64 start_us, qint64 end_us);
	void set_volume(qreal new_volume);

	int channel_count();
	int sample_size();
	int sample_rate();
	QAudioFormat::SampleType sample_type();
	qreal duration();

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
	 * Position of the audio output device has changed.
	 * \param position Position in bytes
	 */
	void play_position_changed(qint64 position);

	void playing();
	void paused();
	void stopped();
	void ended();

private:
	void _reset();
	void _seek_buffer(qint64 new_time);
	qint64 _from_original_to_real_time(qint64 time);
	qint64 _from_real_to_original_time(qint64 time);
	void _set_play_time(qint64 time);

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

    /// Starting play position (in microseconds of the original stream).
    qint64 _start_from_time;
    /// Ending play position (in microseconds of the original stream).
    qint64 _end_at_time;
    /// Current play position (in microseconds of the original stream).
    qint64 _play_time;
    qreal _volume;
    qreal _curr_tempo_change;
    int _curr_pitch_change;
};

} /* namespace cb */

#endif /* SRC_ENGINE_H_ */
