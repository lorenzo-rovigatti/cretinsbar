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
#include "SoundUtils/WavFile.h"

class QAudioOutput;

namespace cb {

class Engine: public QObject {
	Q_OBJECT;

public:
	Engine(QObject *parent);
	virtual ~Engine();

public slots:
	void start_playback();

private:
	void initialise();
	void reset();

private:
	QAudioDeviceInfo _audio_output_device;
	QAudioOutput *_audio_output;
	QAudioFormat _audio_format;
    QBuffer _audio_output_IO_device;
    std::unique_ptr<WavInFile> _wav_file, _out_file;
};

} /* namespace cb */

#endif /* SRC_ENGINE_H_ */
