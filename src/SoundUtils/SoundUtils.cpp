/*
 * SoundUtils.cpp
 *
 *  Created on: 02 feb 2017
 *      Author: lorenzo
 */

#include "SoundUtils.h"

#include "WavFile.h"
#include "Wave.h"

#include <QByteArray>
#include <QDataStream>
#include <QAudioFormat>

namespace cb {

SoundUtils::SoundUtils() {
	pSoundTouch.setSetting(SETTING_USE_QUICKSEEK, 1);
	pSoundTouch.setSetting(SETTING_USE_AA_FILTER, 1);
}

SoundUtils::~SoundUtils() {

}

qint64 SoundUtils::audio_length(QAudioFormat &format, qint64 microseconds) {
	qint64 result = (format.sampleRate() * format.channelCount() * (format.sampleSize() / 8)) * microseconds / 1000000;
	result -= result % (format.channelCount() * format.sampleSize());
	return result;
}

qreal SoundUtils::pcmToReal(QAudioFormat &format, int pcm) {
	qreal max_amplitude = pow(2., format.sampleSize() - 1.);
	return qreal(pcm) / max_amplitude;
}

#define N_SAMPLES 1024
std::unique_ptr<Wave> SoundUtils::process(Wave &in_file, float tempo_change, int pitch_change) {
	int nChannels = (int) in_file.get_channels();

	pSoundTouch.setSampleRate(in_file.get_samples_per_sec());
	pSoundTouch.setChannels(nChannels);

	pSoundTouch.setTempoChange(tempo_change);
	pSoundTouch.setPitchSemiTones(pitch_change);

	float sampleBuffer[N_SAMPLES];

	int samples_per_channel;
	int buffSizeSamples = N_SAMPLES / nChannels;

	std::unique_ptr<Wave> out(new Wave(nChannels, in_file.get_samples_per_sec(), in_file.get_bits_per_sample()));
	// Process samples read from the input file
	for(int i = 0; i < in_file.get_n_samples(); i += N_SAMPLES) {
		// Read a chunk of samples from the input file
		std::vector<float> samples;
		int samples_read = in_file.get_samples(i, N_SAMPLES, samples);
		samples_per_channel = samples_read / nChannels;

		// Feed the samples into SoundTouch processor
		pSoundTouch.putSamples(samples.data(), samples_per_channel);

		// Read ready samples from SoundTouch processor & write them output file.
		// NOTES:
		// - 'receiveSamples' doesn't necessarily return any samples at all
		//   during some rounds!
		// - On the other hand, during some round 'receiveSamples' may have more
		//   ready samples than would fit into 'sampleBuffer', and for this reason
		//   the 'receiveSamples' call is iterated for as many times as it
		//   outputs samples.
		do {
			samples_per_channel = pSoundTouch.receiveSamples(sampleBuffer, buffSizeSamples);
			out->append_samples(sampleBuffer, nChannels * samples_per_channel);
		} while(samples_per_channel != 0);
	}

	// Now the input file is processed, yet 'flush' few last samples that are
	// hiding in the SoundTouch's internal processing pipeline.
	pSoundTouch.flush();
	do {
		samples_per_channel = pSoundTouch.receiveSamples(sampleBuffer, buffSizeSamples);
		out->append_samples(sampleBuffer, nChannels * samples_per_channel);
	} while(samples_per_channel != 0);

	return out;
}

} /* namespace cb */
