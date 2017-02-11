/*
 * SoundUtils.h
 *
 *  Created on: 02 feb 2017
 *      Author: lorenzo
 */

#ifndef SRC_SOUNDUTILS_SOUNDUTILS_H_
#define SRC_SOUNDUTILS_SOUNDUTILS_H_

#include <memory>

#include <QDebug>
#include <QBuffer>

#include <soundtouch/SoundTouch.h>

class QByteArray;
class WavInFile;

namespace cb {

using namespace soundtouch;

class SoundUtils {
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

	std::unique_ptr<WavInFile> process(WavInFile &in_file);

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
};

} /* namespace cb */

#endif /* SRC_SOUNDUTILS_SOUNDUTILS_H_ */
