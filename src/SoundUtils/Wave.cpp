/*
 * Wave.cpp
 *
 *  Created on: 12 feb 2017
 *      Author: lorenzo
 */
#include <stdexcept>
#include <fstream>
#include <errno.h>
#include <cstring>
#include "Wave.h"

#include <QFile>

using namespace cb;

Wave::Wave(const QString &filename) throw (std::exception) {
	_fmt.wFormatTag = 0;
	_extra_param_length = 0;
	_fact.samplesNumber = -1;

	QFile file(filename);
	file.open(QIODevice::ReadOnly);
	if(file.isOpen() == false) {
		throw std::runtime_error(strerror( errno));
	}

	file.read(reinterpret_cast<char*>(&_riff), RIFF_SIZE);
	file.read(reinterpret_cast<char*>(&_fmthdr), FMTHDR_SIZE);

	file.read(reinterpret_cast<char*>(&_fmt), FMT_SIZE);

	unsigned fmt_extra_bytes = _fmthdr.fmtSIZE - FMT_SIZE;

	if(fmt_extra_bytes > 0) {
		_fmt_extra_bytes.resize(fmt_extra_bytes);

		file.read(&_fmt_extra_bytes[0], fmt_extra_bytes);
	}

	if(_fmt.wFormatTag != 1) {
		throw std::runtime_error("Unsupported format: only wav files with format tag == 1 are supported");
	}

	if(get_bytes_per_sample() != 2) {
		throw std::runtime_error("Unsupported format: only 16 bit wav files are currently supported");
	}

	file.read(reinterpret_cast<char*>(&_data.dataID), 4);

	if(_data.dataID[0] == 'f' && _data.dataID[1] == 'a' && _data.dataID[2] == 'c' && _data.dataID[3] == 't') {
		file.read(reinterpret_cast<char*>(&_fact), FACT_SIZE);
		file.read(reinterpret_cast<char*>(&_data), DATA_SIZE);
	}
	else file.read(reinterpret_cast<char*>(&_data.dataSIZE), 4);

	_wave.resize(_data.dataSIZE);

	_wave = file.read(_data.dataSIZE);
}

Wave::Wave() {
	_extra_param_length = 0;
	_fmt.wFormatTag = 0;
	_fact.samplesNumber = -1;
}

Wave::Wave(int16_t nChannels, int32_t nSamplesPerSec, int16_t wBitsPerSample) throw (std::exception) {
	int16_t bytes = (wBitsPerSample + 7) / 8;

	memcpy(_riff.riffID, "RIFF", 4);
	_riff.riffSIZE = 0;

	memcpy(_riff.riffFORMAT, "WAVE", 4);

	memcpy(_fmthdr.fmtID, "fmt ", 4);
	_fmthdr.fmtSIZE = sizeof(FMT);

	_fmt.wFormatTag = 1;
	_fmt.nChannels = nChannels;
	_fmt.nSamplesPerSec = nSamplesPerSec;
	_fmt.nAvgBytesPerSec = nChannels * nSamplesPerSec * bytes;
	_fmt.nBlockAlign = nChannels * bytes;
	_fmt.wBitsPerSample = wBitsPerSample;

	_extra_param_length = 0;
	_fact.samplesNumber = -1;

	memcpy(_data.dataID, "data", 4);
	_data.dataSIZE = 0;

	_update_riff_size();
}

Wave::Wave(const Wave& w) {
	_init(w);
}
Wave& Wave::operator=(const Wave &w) {
	_init(w);
	return *this;
}
Wave::~Wave() {
}

Wave Wave::operator+(const Wave &w) const throw (std::exception) {
	if(_fmt.wFormatTag != w._fmt.wFormatTag) throw std::runtime_error("Can't concatenate waves with different format tags");

	Wave res;
	res._fmthdr = w._fmthdr;
	res._fmt = w._fmt;
	res._fmt_extra_bytes = w._fmt_extra_bytes;

	res._riff = w._riff;
	res._data = w._data;
	res._data.dataSIZE = _data.dataSIZE + w._data.dataSIZE;

	res._extra_param_length = w._extra_param_length;
	if(w._extra_param_length) {
		res._extra_param = w._extra_param;
	}

	res._wave = _wave;

	res._wave.append(w._wave);

	res._update_riff_size();

	return res;
}

Wave& Wave::operator+=(const Wave &w) throw (std::exception) {
	if(_fmt.wFormatTag == 0) {
		_init(w);
		return *this;
	}

	if(_fmt.wFormatTag != w._fmt.wFormatTag) throw std::runtime_error("Can't concatenate waves with different format tags");

	if(_fmt.nChannels != w._fmt.nChannels) throw std::runtime_error("different number of channels");

	if(_fmt.nSamplesPerSec != w._fmt.nSamplesPerSec) throw std::runtime_error("different number of samples per second");

	if(_fmt.wBitsPerSample != w._fmt.wBitsPerSample) throw std::runtime_error("different number of bits per sample");

	_wave.append(w._wave);

	_data.dataSIZE += w._data.dataSIZE;

	_update_riff_size();

	return *this;
}

void Wave::_init(const Wave& w) {
	_fmthdr = w._fmthdr;
	_fmt = w._fmt;
	_fmt_extra_bytes = w._fmt_extra_bytes;
	_riff = w._riff;
	_data = w._data;
	_fact = w._fact;

	_extra_param_length = w._extra_param_length;
	if(w._extra_param_length) _extra_param = w._extra_param;
	_wave = w._wave;
}

int32_t Wave::calc_riff_size(int32_t fmtSIZE, int32_t dataSIZE) {
	return RIFF_SIZE - 4 + FMTHDR_SIZE + fmtSIZE + DATA_SIZE + dataSIZE;
}

void Wave::_update_riff_size() {
	_riff.riffSIZE = calc_riff_size(_fmthdr.fmtSIZE, _data.dataSIZE);
}

void Wave::_update_data_size() {
	_data.dataSIZE = _wave.size();
}

int16_t Wave::get_channels() const {
	return _fmt.nChannels;
}

int16_t Wave::get_bits_per_sample() const {
	return _fmt.wBitsPerSample;
}

int16_t Wave::get_bytes_per_sample() const {
	return get_bits_per_sample() / 8;
}

int32_t Wave::get_samples_per_sec() const {
	return _fmt.nSamplesPerSec;
}

int32_t Wave::get_avg_bytes_per_sec() const {
	return _fmt.nAvgBytesPerSec;
}

int32_t Wave::get_data_size() const {
	return _data.dataSIZE;
}

int32_t Wave::get_n_samples() const {
	return get_data_size() / get_bytes_per_sample();
}

qreal Wave::duration() const {
	return get_n_samples() / (qreal) (get_samples_per_sec()*get_channels());
}

qint64 Wave::bytes_from_us(qint64 us) const {
	qreal time_in_seconds = us / (qreal) 1000000.;
	qint64 n_sample = time_in_seconds * get_samples_per_sec() * get_channels();
	qint64 n_byte = n_sample * get_bytes_per_sample();

	return n_byte;
}

QAudioFormat Wave::format() const {
	QAudioFormat frmt;
	frmt.setChannelCount(get_channels());
	frmt.setSampleRate(get_samples_per_sec());
	frmt.setSampleSize(get_bits_per_sample());
	frmt.setCodec("audio/pcm");
	if(get_bytes_per_sample() == 1) frmt.setSampleType(QAudioFormat::UnSignedInt);
	else frmt.setSampleType(QAudioFormat::SignedInt);
	return frmt;
}

QByteArray *Wave::data() {
	return &_wave;
}

int Wave::get_samples(unsigned int offset, unsigned int n_samples, std::vector<float> &samples) const {
	unsigned int byte_offset = offset * get_bytes_per_sample();
	if(byte_offset > (unsigned) get_data_size()) return 0;

	int byte_size = n_samples * get_bytes_per_sample();
	unsigned int real_size = (byte_offset + byte_size) < (unsigned) get_data_size() ? byte_size : (unsigned) get_data_size() - byte_offset;
	unsigned int real_n_samples = real_size / get_bytes_per_sample();

	switch(get_bytes_per_sample()) {
	case 2: {
		const short *data_short = reinterpret_cast<const short *>(_wave.data() + byte_offset);
		double conv = 1.0 / 32768.0;
		for(uint i = 0; i < real_n_samples; i++) {
			short value_s = data_short[i];
			float value_f = (float) (value_s * conv);
			samples.push_back(value_f);
		}
		break;
	}
	}

	return real_n_samples;
}

void Wave::get_samples(unsigned int offset, unsigned int size, QByteArray &samples) const {
	if(offset > (unsigned) _data.dataSIZE) return;

	unsigned int real_size = (offset + size) < (unsigned) _data.dataSIZE ? size : (unsigned) _data.dataSIZE - offset;

	samples.append(_wave.data() + offset, real_size);
}

int Wave::_saturate(float fvalue, float minval, float maxval) {
	if(fvalue > maxval) fvalue = maxval;
	else if(fvalue < minval) fvalue = minval;
	return (int) fvalue;
}

void Wave::append_samples(const float *samples, int n_samples) {
	int tot_byte = n_samples * get_bytes_per_sample();
	QByteArray to_append(tot_byte, 0);

	switch(get_bytes_per_sample()) {
	case 2: {
		short *data_s = reinterpret_cast<short *>(to_append.data());
		for(int i = 0; i < n_samples; i++) {
			float value_f = samples[i];
			short value_s = (short) _saturate(value_f * 32768.0f, -32768.0f, 32767.0f);
			data_s[i] = value_s;
		}
	}
	}

	_wave.append(to_append);
	_update_data_size();
	_update_riff_size();
}

void Wave::append_samples(const QByteArray &samples) {
	_wave.append(samples);

	_update_data_size();
	_update_riff_size();
}

void Wave::append_samples(const char *samples, int size) {
	_wave.append(samples, size);

	_update_data_size();
	_update_riff_size();
}

void Wave::append_samples(const QByteArray &samples_l, const QByteArray &samples_r) {
	if(_fmt.nChannels != 2) throw std::logic_error(("Wave::append_samples(): cannot add stereo samples, nChannels = " + std::to_string(_fmt.nChannels)).c_str());

	if(samples_l.size() != samples_r.size()) throw std::logic_error(("Wave::append_samples(): samples have different sizes, l " + std::to_string(samples_l.size()) + " r " + std::to_string(samples_r.size())).c_str());

	int bytes_per_sample = _fmt.wBitsPerSample / 8;

	size_t size = samples_l.size();

	for(size_t i = 0; i < size; i = i + bytes_per_sample) {
		_wave.append(samples_l.data() + i, bytes_per_sample);
		_wave.append(samples_r.data() + i, bytes_per_sample);
	}

	_update_data_size();
	_update_riff_size();
}

void Wave::append_samples(const char* samples_l, const char* samples_r, int size) {
	if(_fmt.nChannels != 2) throw std::logic_error(("Wave::append_samples(): cannot add stereo samples, nChannels = " + std::to_string(_fmt.nChannels)).c_str());

	int bytes_per_sample = _fmt.wBitsPerSample / 8;

	for(int i = 0; i < size; i = i + bytes_per_sample) {
		_wave.append(samples_l + i, bytes_per_sample);
		_wave.append(samples_r + i, bytes_per_sample);
	}

	_update_data_size();
	_update_riff_size();
}

void Wave::save(const QString &filename) {
	QFile file(filename);
	file.open(QIODevice::WriteOnly);

	file.write(reinterpret_cast<char*>(&_riff), RIFF_SIZE);
	file.write(reinterpret_cast<char*>(&_fmthdr), FMTHDR_SIZE);

	file.write(reinterpret_cast<char*>(&_fmt), FMT_SIZE);
	file.write(&_fmt_extra_bytes[0], _fmt_extra_bytes.size());

	if(_fmt.wFormatTag > 1) {
		file.write(reinterpret_cast<char*>(&_extra_param_length), 2);
		if(_extra_param_length > 0) file.write(&_extra_param[0], _extra_param_length);
	}
	if(_fact.samplesNumber > -1) {
		file.write(const_cast<char*>("fact"), 4);
		file.write(reinterpret_cast<char*>(&_fact), FACT_SIZE);
	}

	file.write(reinterpret_cast<char*>(&_data), DATA_SIZE);
	file.write(_wave.data(), _data.dataSIZE);
}
