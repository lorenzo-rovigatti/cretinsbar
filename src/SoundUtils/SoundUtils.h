/*
 * SoundUtils.h
 *
 *  Created on: 02 feb 2017
 *      Author: lorenzo
 */

#ifndef SRC_SOUNDUTILS_SOUNDUTILS_H_
#define SRC_SOUNDUTILS_SOUNDUTILS_H_

#include <QDebug>
#include <QBuffer>

#include <soundtouch/SoundTouch.h>

namespace cb {

using namespace soundtouch;

class SoundUtils {
private:
	soundtouch::SoundTouch pSoundTouch;

	SoundUtils(SoundUtils const&) {
	}
	SoundUtils& operator=(SoundUtils const&);
protected:

	virtual ~SoundUtils() {

	}

	SoundUtils() {

	}

public:
	static SoundUtils* Instance() {
		static SoundUtils* m_pInstance;
		if(!m_pInstance)
			m_pInstance = new SoundUtils;
		assert(m_pInstance != NULL);
		return m_pInstance;
	}

	/**
	 * setup SoundTouch to change tempo and/or sample rate
	 * @param inSampleRate the incoming sample rate
	 * @param inChannels the number of channels
	 * @param tempChange the tempo change in percent
	 * @param outSampleRate the desired output sample rate
	 */
	void setup(int inSampleRate, int inChannels, float tempoChange, int outSampleRate);

	/**
	 * process the samples according to the setup (change tempo and/or samples rate)
	 * @param samples the samples to process S16_LE
	 * @param nSamples the number of samples
	 * @param nChannels the number of channels
	 * @param sampleSizeBytes the size of one sample in bytes
	 * @param runningBuf the output buffer
	 */
	int process(const float *samples, int nSamples, int nChannels, int sampleSizeBytes, QBuffer* runningBuf);

	/**
	 * trim the silence at the beginning and end
	 * @param in the incoming buffer
	 * @param numsamples the number of PCM samples
	 * @param out the output buffer
	 */
	int trim(QBuffer* in, int numsamples, QBuffer* out);
};

} /* namespace cb */

#endif /* SRC_SOUNDUTILS_SOUNDUTILS_H_ */
