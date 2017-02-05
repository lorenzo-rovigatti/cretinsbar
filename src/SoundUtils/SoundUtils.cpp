/*
 * SoundUtils.cpp
 *
 *  Created on: 02 feb 2017
 *      Author: lorenzo
 */

#include "SoundUtils.h"

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

	if(inSampleRate != outSampleRate)
		pSoundTouch.setRate((float) inSampleRate / (float) outSampleRate);

	pSoundTouch.setSetting(SETTING_USE_QUICKSEEK, 1);

	{
		// use settings for speech processing
		pSoundTouch.setSetting(SETTING_SEQUENCE_MS, 40);
		pSoundTouch.setSetting(SETTING_SEEKWINDOW_MS, 15);
		pSoundTouch.setSetting(SETTING_OVERLAP_MS, 8);
	}
}

#define BUFF_SIZE 16384

// Processes the sound
int SoundUtils::process(const float *samples, int nSamples, int nChannels, int sampleSizeBytes, QBuffer *runningBuf) {
	int buffSizeSamples;

	SAMPLETYPE* sampleBuffer = new SAMPLETYPE[BUFF_SIZE];

	assert(nChannels > 0);
	buffSizeSamples = BUFF_SIZE / nChannels;
	int totSamples;

	// Process samples
	{
		pSoundTouch.putSamples(samples, nSamples);

		do {
			nSamples = pSoundTouch.receiveSamples(sampleBuffer, buffSizeSamples);
			totSamples += nSamples;
//            qDebug() << "proccessed " << nSamples;
			runningBuf->write(reinterpret_cast<char*>(sampleBuffer), nSamples *sampleSizeBytes);
//            qDebug() << "runningBuf " << runningBuf->size();
		} while(nSamples != 0);
	}

	pSoundTouch.flush();
	do {
		nSamples = pSoundTouch.receiveSamples(sampleBuffer, buffSizeSamples);
		totSamples += nSamples;
		runningBuf->write(reinterpret_cast<char*>(sampleBuffer), nSamples *sampleSizeBytes);

//        qDebug() << "runningBuf " << runningBuf->size();
	} while(nSamples != 0);

	delete sampleBuffer;

//    qDebug() << "done processing";

	return totSamples;
}

int SoundUtils::trim(QBuffer* in, int numsamples, QBuffer* out) {
	int i = 0;
	in->open(QIODevice::ReadOnly);
	const short* pBuf = reinterpret_cast<const short*>(in->data().data());
	for(i = 0; i < numsamples && abs(pBuf[i]) <= 10; ++i)
		;    // qDebug() << pBuf[i];
	qDebug() << "start silence " << i;
	int start_silence = i;
	for(i = numsamples - 1; i >= 0 && abs(pBuf[i]) <= 10; --i)
		;    // qDebug() << pBuf[i];
	qDebug() << "end silence " << i;
	int end_silence = i;
	out->open(QIODevice::WriteOnly);
	out->write(in->data().mid(start_silence * sizeof(short), (end_silence - start_silence) * sizeof(short)));
	return end_silence - start_silence;
}

} /* namespace cb */
