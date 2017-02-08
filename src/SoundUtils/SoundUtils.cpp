/*
 * SoundUtils.cpp
 *
 *  Created on: 02 feb 2017
 *      Author: lorenzo
 */

#include "SoundUtils.h"

#include "WavFile.h"

#include <QByteArray>
#include <QDataStream>

namespace cb {

// Sets the 'SoundTouch' object up according to parameters
void SoundUtils::setup(int inSampleRate, int inChannels, float tempoChange, int outSampleRate) {
	static int last_inSampleRate = -1;
	static int last_inChannels = -1;
	static int last_tempChange = 0.0f;
	static int last_outSampleRate = -1;

	if(inSampleRate == last_inSampleRate && inChannels == last_inChannels && tempoChange == last_tempChange && outSampleRate == last_outSampleRate) {
		return; //parameters are the same - no need to 'setup' again
	}

	last_inSampleRate = inSampleRate;
	last_inChannels = inChannels;
	last_tempChange = tempoChange;
	last_outSampleRate = outSampleRate;

	pSoundTouch.setSampleRate(inSampleRate);
	pSoundTouch.setChannels(inChannels);

	pSoundTouch.setTempoChange(tempoChange);
	pSoundTouch.setPitchSemiTones(0);

	pSoundTouch.setSetting(SETTING_USE_QUICKSEEK, 1);
}

/// Convert from float to integer and saturate
inline int my_saturate(float fvalue, float minval, float maxval) {
	if(fvalue > maxval) {
		fvalue = maxval;
	}
	else if(fvalue < minval) {
		fvalue = minval;
	}
	return (int) fvalue;
}


#define BUFF_SIZE 6720
void SoundUtils::process(WavInFile &in_file, QByteArray *runningBuf) {
	runningBuf->clear();

	int nSamples;
	float sampleBuffer[BUFF_SIZE];
	int sampleIntBuffer[BUFF_SIZE];

	int nChannels = (int) in_file.numChannels();
	assert(nChannels > 0);
	int buffSizeSamples = BUFF_SIZE / nChannels;

	QDataStream out_buff(runningBuf, QIODevice::WriteOnly);
	// Process samples read from the input file
	while(in_file.eof() == 0) {
		int num;

		// Read a chunk of samples from the input file
		num = in_file.read(sampleBuffer, BUFF_SIZE);
		nSamples = num / nChannels;
//		out_buff.writeRawData(reinterpret_cast<const char *>(sampleIntBuffer), in_file.getBytesPerSample()*nSamples);

		// Feed the samples into SoundTouch processor
		pSoundTouch.putSamples(sampleBuffer, nSamples);

		// Read ready samples from SoundTouch processor & write them output file.
		// NOTES:
		// - 'receiveSamples' doesn't necessarily return any samples at all
		//   during some rounds!
		// - On the other hand, during some round 'receiveSamples' may have more
		//   ready samples than would fit into 'sampleBuffer', and for this reason
		//   the 'receiveSamples' call is iterated for as many times as it
		//   outputs samples.
		do {
			nSamples = pSoundTouch.receiveSamples(sampleBuffer, buffSizeSamples);
			for(int i = 0; i < nSamples*2; i++) {
				sampleIntBuffer[i] = my_saturate(sampleBuffer[i]*2147483648.0f, -2147483648.0f, 2147483647.0f);
			}
			out_buff.writeRawData(reinterpret_cast<const char *>(sampleIntBuffer), in_file.bytesPerSampleTimesNumChannels()*nSamples);
		} while(nSamples != 0);
	}

	// Now the input file is processed, yet 'flush' few last samples that are
	// hiding in the SoundTouch's internal processing pipeline.
	pSoundTouch.flush();
	do {
		nSamples = pSoundTouch.receiveSamples(sampleBuffer, buffSizeSamples);
		for(int i = 0; i < nSamples*2; i++) {
			sampleIntBuffer[i] = my_saturate(sampleBuffer[i]*2147483648.0f, -2147483648.0f, 2147483647.0f);
		}
		out_buff.writeRawData(reinterpret_cast<const char *>(sampleIntBuffer), in_file.bytesPerSampleTimesNumChannels()*nSamples);
	} while(nSamples != 0);
}

} /* namespace cb */
