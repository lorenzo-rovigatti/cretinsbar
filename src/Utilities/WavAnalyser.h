/*
 * WavAnalyser.h
 *
 *  Created on: 04 feb 2017
 *      Author: lorenzo
 */

#ifndef SRC_UTILITIES_WAVANALYSER_H_
#define SRC_UTILITIES_WAVANALYSER_H_

#include <QAudioFormat>

namespace cb {

class WavAnalyser {
private:
	WavAnalyser();
	virtual ~WavAnalyser();

public:
	static QAudioFormat format(QByteArray &);
};

} /* namespace cb */

#endif /* SRC_UTILITIES_WAVANALYSER_H_ */
