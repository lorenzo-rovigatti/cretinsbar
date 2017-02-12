/*
 * Wave.h
 *
 *  Created on: 12 feb 2017
 *      Author: lorenzo
 */

#ifndef SRC_SOUNDUTILS_WAVE_H
#define SRC_SOUNDUTILS_WAVE_H

#include <QAudioFormat>
#include <QByteArray>

#include <string>
#include <vector>       // std::vector
#include <stdint.h>     // int32_t, int16_t
#include <exception>    // std::exception

namespace cb {

class Wave {
public:
	Wave();
	Wave(const std::string & filename) throw (std::exception);
	Wave(const Wave& w);

	Wave(int16_t nChannels, int32_t nSamplesPerSec, int16_t wBitsPerSample) throw (std::exception);

	virtual ~Wave();

	Wave operator+(const Wave& wave) const throw (std::exception);
	Wave& operator+=(const Wave& wave) throw (std::exception);
	Wave& operator=(const Wave &w);

	int16_t get_channels() const;
	int16_t get_bits_per_sample() const;
	int16_t get_bytes_per_sample() const;
	int32_t get_samples_per_sec() const;
	int32_t get_avg_bytes_per_sec() const;
	int32_t get_data_size() const;
	int32_t get_n_samples() const;
	QAudioFormat format() const;
	QByteArray *data();

	int get_samples(unsigned int offset, unsigned int n_samples, std::vector<float> &samples) const;
	void get_samples(unsigned int offset, unsigned int size, QByteArray &samples) const;

	void append_samples(const float *samples, int n_samples);
	void append_samples(const QByteArray &samples);
	void append_samples(const char* samples, int size);
	void append_samples(const QByteArray &samples_l, const QByteArray &samples_r);
	void append_samples(const char* samples_l, const char* samples_r, int size);

	void save(const QString &filename);

	struct RIFF {
		char riffID[4];     //4
		int32_t riffSIZE;   //4
		char riffFORMAT[4]; //4
	};

	struct FMTHDR {
		char fmtID[4];      //4
		int32_t fmtSIZE;    //4
	};

	struct FMT {
		int16_t wFormatTag; //2
		int16_t nChannels;  //2
		int32_t nSamplesPerSec;     //4
		int32_t nAvgBytesPerSec;    //4
		int16_t nBlockAlign;        //2
		int16_t wBitsPerSample;     //2
	};

	struct DATA {
		char dataID[4];     // 4 bytes
		int32_t dataSIZE;   // 4 bytes
	};
	struct FACT {
		int32_t samplesNumber;
		int32_t t;
	};

public:
	static const int RIFF_SIZE = 12;
	static const int FMTHDR_SIZE = 8;
	static const int FMT_SIZE = 16;
	static const int DATA_SIZE = 8;
	static const int FACT_SIZE = 8;

private:
	void _init(const Wave&);

	static int32_t calc_riff_size(int32_t fmtSIZE, int32_t dataSIZE);
	void _update_riff_size();
	void _update_data_size();

	inline int _saturate(float fvalue, float minval, float maxval);

private:
	QByteArray _wave;

	RIFF _riff;
	FMTHDR _fmthdr;
	FMT _fmt;
	std::vector<char> _fmt_extra_bytes;
	FACT _fact;
	DATA _data;
	int16_t _extra_param_length;
	std::vector<char> _extra_param;

};

} // namespace cb

#endif // SRC_SOUNDUTILS_WAVE_H
