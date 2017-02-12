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

class Wave;
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

	std::unique_ptr<Wave> process(Wave &in_file, float tempo_change, int pitch_change);

private:
	soundtouch::SoundTouch pSoundTouch;

	SoundUtils();

	SoundUtils(SoundUtils const&) = delete;
	SoundUtils& operator=(SoundUtils const&) = delete;

	virtual ~SoundUtils();
};

} /* namespace cb */

#endif /* SRC_SOUNDUTILS_SOUNDUTILS_H_ */
