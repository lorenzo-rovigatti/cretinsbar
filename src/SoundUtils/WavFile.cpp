////////////////////////////////////////////////////////////////////////////////
///
/// Classes for easy reading & writing of WAV sound files. 
///
/// For big-endian CPU, define _BIG_ENDIAN_ during compile-time to correctly
/// parse the WAV files with such processors.
/// 
/// Admittingly, more complete WAV reader routines may exist in public domain,
/// but the reason for 'yet another' one is that those generic WAV reader 
/// libraries are exhaustingly large and cumbersome! Wanted to have something
/// simpler here, i.e. something that's not already larger than rest of the
/// SoundTouch/SoundStretch program...
///
/// Author        : Copyright (c) Olli Parviainen
/// Author e-mail : oparviai 'at' iki.fi
/// SoundTouch WWW: http://www.surina.net/soundtouch
///
////////////////////////////////////////////////////////////////////////////////
//
// Last changed  : $Date: 2014-10-05 19:20:24 +0300 (Sun, 05 Oct 2014) $
// File revision : $Revision: 4 $
//
// $Id: WavFile.cpp 200 2014-10-05 16:20:24Z oparviai $
//
////////////////////////////////////////////////////////////////////////////////
//
// License :
//
//  SoundTouch audio processing library
//  Copyright (c) Olli Parviainen
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string>
#include <sstream>
#include <cstring>
#include <assert.h>
#include <limits.h>

#include <QFile>
#include <QDebug>
#include <QString>

#include "WavFile.h"
#include <soundtouch/STTypes.h>

using namespace std;

static const QByteArray riffStr("RIFF");
static const QByteArray waveStr("WAVE");
static const QByteArray fmtStr("fmt ");
static const QByteArray factStr("fact");
static const QByteArray dataStr("data");

//////////////////////////////////////////////////////////////////////////////
//
// Helper functions for swapping byte order to correctly read/write WAV files 
// with big-endian CPU's: Define compile-time definition _BIG_ENDIAN_ to
// turn-on the conversion if it appears necessary. 
//
// For example, Intel x86 is little-endian and doesn't require conversion,
// while PowerPC of Mac's and many other RISC cpu's are big-endian.

#ifdef BYTE_ORDER
// In gcc compiler detect the byte order automatically
#if BYTE_ORDER == BIG_ENDIAN
// big-endian platform.
#define _BIG_ENDIAN_
#endif
#endif

#ifdef _BIG_ENDIAN_
// big-endian CPU, swap bytes in 16 & 32 bit words

// helper-function to swap byte-order of 32bit integer
static inline int _swap32(int &dwData)
{
	dwData = ((dwData >> 24) & 0x000000FF) |
	((dwData >> 8) & 0x0000FF00) |
	((dwData << 8) & 0x00FF0000) |
	((dwData << 24) & 0xFF000000);
	return dwData;
}

// helper-function to swap byte-order of 16bit integer
static inline short _swap16(short &wData)
{
	wData = ((wData >> 8) & 0x00FF) |
	((wData << 8) & 0xFF00);
	return wData;
}

// helper-function to swap byte-order of buffer of 16bit integers
static inline void _swap16Buffer(short *pData, int numWords)
{
	int i;

	for (i = 0; i < numWords; i ++)
	{
		pData[i] = _swap16(pData[i]);
	}
}

#else   // BIG_ENDIAN
// little-endian CPU, WAV file is ok as such

// dummy helper-function
static inline int _swap32(int &dwData) {
	// do nothing
	return dwData;
}

// dummy helper-function
static inline short _swap16(short &wData) {
	// do nothing
	return wData;
}

#endif  // BIG_ENDIAN

//////////////////////////////////////////////////////////////////////////////
//
// Class WavInFile
//

WavInFile::WavInFile(const char *fileName) {
	// Try to open the file for reading

	_data_file.setFileName(fileName);
	if(!_data_file.open(QIODevice::ReadOnly)) {
		// didn't succeed
		string msg = "Error : Unable to open file \"";
		msg += fileName;
		msg += "\" for reading.";
		ST_THROW_RT_ERROR(msg.c_str());
	}

	init();
}

WavInFile::WavInFile(WavHeader &&header)
	: _header(header)
	, _position(0)
	, _curr_sample(0)
	, _bytes_written(0) {
}

/// Init the WAV file stream
void WavInFile::init() {
	int hdrsOk;

	// assume file stream is already open
	Q_ASSERT(_data_file.isOpen());

	// Read the file headers
	hdrsOk = _read_wav_headers();
	if(hdrsOk != 0) {
		// Something didn't match in the wav file headers
		string msg = "Input file is corrupt or not a WAV file";
		ST_THROW_RT_ERROR(msg.c_str());
	}

	_curr_sample = 0;

	_data = _data_file.read(_header.data.data_len);
	if((uint)_data.size() != (uint)_header.data.data_len) ST_THROW_RT_ERROR("The actual size of the wav file is different from what it should be according to its header.");
	float *float_data = reinterpret_cast<float *>(_data.data());

	// swap byte ordert & convert to float, depending on sample format
	switch(bytes_per_sample()) {
	case 1: {
		unsigned char *temp2 = reinterpret_cast<unsigned char *>(_data.data());
		double conv = 1.0 / 128.0;
		for(uint i = 0; i < num_samples(); i++) {
			float_data[i] = (float) (temp2[i] * conv - 1.0);
		}
		break;
	}

	case 2: {
		short *temp2 = reinterpret_cast<short *>(_data.data());
		double conv = 1.0 / 32768.0;
		for(uint i = 0; i < num_samples(); i++) {
			short value = temp2[i];
			float_data[i] = (float) (_swap16(value) * conv);
		}
		break;
	}

	case 3: {
		char *temp2 = reinterpret_cast<char *>(_data.data());
		double conv = 1.0 / 8388608.0;
		for(uint i = 0; i < num_samples(); i++) {
			int value = *((int*) temp2);
			value = _swap32(value) & 0x00ffffff;             // take 24 bits
			value |= (value & 0x00800000) ? 0xff000000 : 0;  // extend minus sign bits
			float_data[i] = (float) (value * conv);
			temp2 += 3;
		}
		break;
	}

	case 4: {
		int *temp2 = reinterpret_cast<int *>(_data.data());
		double conv = 1.0 / 2147483648.0;
		assert(sizeof(int) == 4);
		for(uint i = 0; i < num_samples(); i++) {
			int value = temp2[i];
			float_data[i] = (float) (_swap32(value) * conv);
		}
		break;
	}
	}
}

WavInFile::~WavInFile() {
	if(_data_file.isOpen()) _data_file.close();
}

void WavInFile::rewind() {
	_data_file.seek(0);
	int hdrsOk = _read_wav_headers();
	Q_UNUSED(hdrsOk);
	assert(hdrsOk == 0);
	_curr_sample = 0;
}

int WavInFile::_check_string_tags() const {
	// header.format.fmt should equal to 'fmt '
	if(fmtStr != _header.format.fmt) return -1;
	// header.data.data_field should equal to 'data'
	if(dataStr != _header.data.data_field)	return -1;

	return 0;
}

/// Read data in float format. Notice that when reading in float format 
/// 8/16/24/32 bit sample formats are supported
int WavInFile::read(float *buffer, int maxElems) {
	int numElems = maxElems;
	float *float_data = reinterpret_cast<float *>(_data.data());

	if((bytes_per_sample() < 1) || (bytes_per_sample() > 4)) {
		stringstream ss;
		ss << "\nOnly 8/16/24/32 bit sample WAV files supported. Can't open WAV file with ";
		ss << (int) _header.format.bits_per_sample;
		ss << " bit sample format. ";
		ST_THROW_RT_ERROR(ss.str().c_str());
	}

	if((_curr_sample + maxElems) > num_samples()) numElems = num_samples() - _curr_sample;
	for(int i = 0; i < numElems; i++) buffer[i] = float_data[_curr_sample + i];
	_curr_sample += numElems;

	return numElems;
}

bool WavInFile::eof() const {
	// return true if all data has been read or file eof has reached
	return (_curr_sample == num_samples() || _data_file.atEnd());
}

// test if character code is between a white space ' ' and little 'z'
static int isAlpha(char c) {
	return (c >= ' ' && c <= 'z') ? 1 : 0;
}

// test if all characters are between a white space ' ' and little 'z'
static int isAlphaStr(const char *str) {
	char c;

	c = str[0];
	while(c) {
		if(isAlpha(c) == 0)
			return 0;
		str++;
		c = str[0];
	}

	return 1;
}

int WavInFile::_read_RIFF_block() {
	_header.riff.riff = _data_file.read(4);

	QByteArray temp_len = _data_file.read(4);
	memcpy(&_header.riff.package_len, temp_len.constData(), 4);
	_header.riff.wave = _data_file.read(4);

	// swap 32bit data byte order if necessary
	_swap32((int &) _header.riff.package_len);

	// header.riff.riff should equal to 'RIFF');
	if(riffStr != _header.riff.riff) return -1;
	// header.riff.wave should equal to 'WAVE'
	if(waveStr != _header.riff.wave) return -1;

	return 0;
}

int WavInFile::_read_header_block() {
	string sLabel;

	// lead label string
	QByteArray label = _data_file.read(4);
	if(label.size() != 4) return -1;

	if(isAlphaStr(label) == 0) return -1;    // not a valid label

	// Decode blocks according to their label
	if(label == fmtStr) {
		int nLen, nDump;

		// 'fmt ' block
		_header.format.fmt = fmtStr;

		// read length of the format field
		if(_data_file.read((char *) &nLen, sizeof(int)) != sizeof(int)) return -1;
		// swap byte order if necessary
		_swap32(nLen); // int format_len;
		_header.format.format_len = nLen;

		// calculate how much length differs from expected
		nDump = nLen - ((int) sizeof(_header.format) - 8);

		// if format_len is larger than expected, read only as much data as we've space for
		if(nDump > 0) {
			nLen = sizeof(_header.format) - 8;
		}

		// read data
		if(_data_file.read((char *) &(_header.format.fixed), nLen) != nLen) return -1;

		// swap byte order if necessary
		_swap16(_header.format.fixed);            // short int fixed;
		_swap16(_header.format.channel_number);   // short int channel_number;
		_swap32((int &) _header.format.sample_rate);      // int sample_rate;
		_swap32((int &) _header.format.byte_rate);        // int byte_rate;
		_swap16(_header.format.bytes_per_sample);  // short int byte_per_sample;
		_swap16(_header.format.bits_per_sample);  // short int bits_per_sample;

		// if format_len is larger than expected, skip the extra data
		if(nDump > 0) _data_file.seek(_data_file.pos() + nDump);

		return 0;
	}
	else if(label == factStr) {
		int nLen, nDump;

		// 'fact' block
		_header.fact.fact_field = factStr;

		// read length of the fact field
		if(_data_file.read((char *) &nLen, sizeof(int)) != sizeof(int)) return -1;
		// swap byte order if necessary
		_swap32(nLen); // int fact_len;
		_header.fact.fact_len = nLen;

		// calculate how much length differs from expected
		nDump = nLen - ((int) sizeof(_header.fact) - 8);

		// if format_len is larger than expected, read only as much data as we've space for
		if(nDump > 0) {
			nLen = sizeof(_header.fact) - 8;
		}

		// read data
		if(_data_file.read((char *) &(_header.fact.fact_sample_len), nLen) != nLen) return -1;

		// swap byte order if necessary
		_swap32((int &) _header.fact.fact_sample_len);    // int sample_length;

		// if fact_len is larger than expected, skip the extra data
		if(nDump > 0) _data_file.seek(_data_file.pos() + nDump);

		return 0;
	}
	else if(label == dataStr) {
		// 'data' block
		_header.data.data_field = dataStr;
		if(_data_file.read((char *) &(_header.data.data_len), sizeof(uint)) != sizeof(uint)) return -1;

		// swap byte order if necessary
		_swap32((int &) _header.data.data_len);

		return 1;
	}
	else {
		uint len, i;
		uint temp;
		// unknown block

		// read length
		if(_data_file.read((char *) &len, sizeof(len)) != sizeof(len)) return -1;
		// scan through the block
		for(i = 0; i < len; i++) {
			if(_data_file.read((char *) &temp, 1) != 1) return -1;
			if(_data_file.atEnd()) return -1; // unexpected eof
		}
	}
	return 0;
}

int WavInFile::_read_wav_headers() {
	int res;

	res = _read_RIFF_block();
	if(res)
		return 1;
	// read header blocks until data block is found
	do {
		// read header blocks
		res = _read_header_block();
		if(res < 0)
			return 1;  // error in file structure
	} while(res == 0);
	// check that all required tags are legal
	return _check_string_tags();
}

QByteArray *WavInFile::data() {
	return &_data;
}

WavHeader WavInFile::header() const {
	return _header;
}

uint WavInFile::num_channels() const {
	return _header.format.channel_number;
}

uint WavInFile::num_bits() const {
	return _header.format.bits_per_sample;
}

uint WavInFile::bytes_per_sample() const {
	return num_bits()/8;
}

uint WavInFile::bytes_per_sample_times_num_channels() const {
	return bytes_per_sample()*num_channels();
}

uint WavInFile::sample_rate() const {
	return _header.format.sample_rate;
}

uint WavInFile::data_size_in_bytes() const {
	return _header.data.data_len;
}

uint WavInFile::num_samples() const {
	if(_header.format.bytes_per_sample == 0) return 0;
	if(_header.format.fixed > 1) return 2*_header.fact.fact_sample_len;
	return _header.data.data_len/(unsigned short) bytes_per_sample();
}

uint WavInFile::num_samples_per_channel() const {
	return num_samples()/num_channels();
}

uint WavInFile::length_in_ms() const {
	return (uint) (1000.0*num_samples_per_channel()/sample_rate() + 0.5);
}

QAudioFormat WavInFile::format() const {
	QAudioFormat frmt;
	frmt.setChannelCount(num_channels());
	frmt.setSampleRate(sample_rate());
	frmt.setSampleSize(num_bits());
	frmt.setCodec("audio/pcm");
	frmt.setSampleType(QAudioFormat::Float);
	return frmt;
}

/// Returns how many milliseconds of audio have so far been read from the file
uint WavInFile::elapes_in_ms() const {
	return (uint) (1000.0*(double) _curr_sample/(double) _header.format.byte_rate);
}

void WavInFile::_finish_header() {
	// supplement the file length into the header structure
	_header.riff.package_len = _bytes_written + sizeof(WavHeader) - sizeof(WavRiff) + 4;
	_header.data.data_len = _bytes_written;
	_header.fact.fact_sample_len = _bytes_written / _header.format.bytes_per_sample;

	_write_header();
}

void WavInFile::_write_header() {
	WavHeader hdrTemp;
	int res;

	// swap byte order if necessary
	hdrTemp = _header;
	_swap32((int &) hdrTemp.riff.package_len);
	_swap32((int &) hdrTemp.format.format_len);
	_swap16((short &) hdrTemp.format.fixed);
	_swap16((short &) hdrTemp.format.channel_number);
	_swap32((int &) hdrTemp.format.sample_rate);
	_swap32((int &) hdrTemp.format.byte_rate);
	_swap16((short &) hdrTemp.format.bytes_per_sample);
	_swap16((short &) hdrTemp.format.bits_per_sample);
	_swap32((int &) hdrTemp.data.data_len);
	_swap32((int &) hdrTemp.fact.fact_len);
	_swap32((int &) hdrTemp.fact.fact_sample_len);

	// write the supplemented header in the beginning of the file
	fseek(fptr, 0, SEEK_SET);
	res = (int) fwrite(&hdrTemp, sizeof(hdrTemp), 1, fptr);
	if(res != 1) {
		ST_THROW_RT_ERROR("Error while writing to a wav file.");
	}

	// jump back to the end of the file
	fseek(fptr, 0, SEEK_END);
}

/// Convert from float to integer and saturate
inline int saturate(float fvalue, float minval, float maxval) {
	if(fvalue > maxval) {
		fvalue = maxval;
	}
	else if(fvalue < minval) {
		fvalue = minval;
	}
	return (int) fvalue;
}

void WavInFile::_prepare_to_write_data() {
	_data_stream = std::unique_ptr<QDataStream>(new QDataStream(&_data, QIODevice::WriteOnly));
	_bytes_written = 0;
}

void WavInFile::write(const float *buffer, int numElems) {
	if(_data_stream.get() == nullptr) _prepare_to_write_data();

	if(numElems == 0) return;

	int numBytes = numElems*bytes_per_sample();
	char *temp = (char *) (new char[numBytes]);

	switch(bytes_per_sample()) {
	case 1: {
		unsigned char *temp2 = (unsigned char *) temp;
		for(int i = 0; i < numElems; i++) {
			temp2[i] = (unsigned char) saturate(buffer[i] * 128.0f + 128.0f, 0.0f, 255.0f);
		}
		break;
	}

	case 2: {
		short *temp2 = (short *) temp;
		for(int i = 0; i < numElems; i++) {
			short value = (short) saturate(buffer[i] * 32768.0f, -32768.0f, 32767.0f);
			temp2[i] = _swap16(value);
		}
		break;
	}

	case 3: {
		char *temp2 = (char *) temp;
		for(int i = 0; i < numElems; i++) {
			int value = saturate(buffer[i] * 8388608.0f, -8388608.0f, 8388607.0f);
			*((int*) temp2) = _swap32(value);
			temp2 += 3;
		}
		break;
	}

	case 4: {
		int *temp2 = (int *) temp;
		for(int i = 0; i < numElems; i++) {
			int value = saturate(buffer[i] * 2147483648.0f, -2147483648.0f, 2147483647.0f);
			temp2[i] = _swap32(value);
		}
		break;
	}

	default:
		assert(false);
	}

	int res = _data_stream->writeRawData(temp, numBytes);

	if(res != numBytes) {
		ST_THROW_RT_ERROR("Error while writing to a wav file.");
	}

	_bytes_written += numBytes;
	qDebug() << _bytes_written;
}

void WavInFile::done_writing_data() {
	_data_stream.reset();
}
