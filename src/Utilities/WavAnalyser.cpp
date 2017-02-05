/*
 * WavAnalyser.cpp
 *
 *  Created on: 04 feb 2017
 *      Author: lorenzo
 */

#include "WavAnalyser.h"

#include <QDataStream>
#include <QDebug>
#include <QAudioFormat>

namespace cb {

WavAnalyser::WavAnalyser() {
	// TODO Auto-generated constructor stub

}

WavAnalyser::~WavAnalyser() {
	// TODO Auto-generated destructor stub
}

/** Read a wav file to play audio into a buffer and return the size of the data read
 * after stripping the header.
 *
 * The header for a WAV file looks like this:
 * Positions	Sample Value	Description
 * 1 - 4	"RIFF"	Marks the file as a riff file. Characters are each 1 byte long.
 * 5 - 8	File size (integer)	Size of the overall file - 8 bytes, in bytes (32-bit integer).
 * 9 -12	"WAVE"	File Type Header. For our purposes, it always equals "WAVE".
 * 13-16	"fmt "	Format chunk marker. Includes trailing null
 * 17-20	16	Length of format data as listed above
 * 21-22	1	Type of format (1 is PCM) - 2 byte integer
 * 23-24	2	Number of Channels - 2 byte integer
 * 25-28	44100	Sample Rate - 32 byte integer. CSample Rate = Number of Samples per second, or Hertz.
 * 29-32	176400	(Sample Rate * BitsPerSample * Channels) / 8.
 * 33-34	4	(BitsPerSample * Channels) / 8.1 - 8 bit mono2 - 8 bit stereo/16 bit mono4 - 16 bit stereo
 * 35-36	16	Bits per sample
 * 37-40	"data"	"data" chunk header. Marks the beginning of the data section.
 * 41-44	File size (data)	Size of the data section.
 * Sample values are given above for a 16-bit stereo source.
 *
 * @param fileName
 * A QString representing the file location to open with QFile
 *
 */

QAudioFormat WavAnalyser::format(QByteArray &wavFileContent) {
	qDebug() << "The size of the WAV file is: " << wavFileContent.size();
	// Define the header components
	char fileType[4];
	qint32 fileSize;
	char waveName[4];
	char fmtName[4];
	qint32 fmtLength;
	short fmtType;
	short numberOfChannels;
	qint32 sampleRate;
	qint32 sampleRateXBitsPerSampleXChanngelsDivEight;
	short bitsPerSampleXChannelsDivEightPointOne;
	short bitsPerSample;
	char dataHeader[4];
	qint32 dataSize;

	// Create a data stream to analyze the data
	QDataStream analyzeHeaderDS(&wavFileContent, QIODevice::ReadOnly);
	analyzeHeaderDS.setByteOrder(QDataStream::LittleEndian);
	// Now pop off the appropriate data into each header field defined above
	analyzeHeaderDS.readRawData(fileType, 4); // "RIFF"
	analyzeHeaderDS >> fileSize; // File Size
	analyzeHeaderDS.readRawData(waveName, 4); // "WAVE"
	analyzeHeaderDS.readRawData(fmtName, 4); // "fmt"
	analyzeHeaderDS >> fmtLength; // Format length
	analyzeHeaderDS >> fmtType; // Format type
	analyzeHeaderDS >> numberOfChannels; // Number of channels
	analyzeHeaderDS >> sampleRate; // Sample rate
	analyzeHeaderDS >> sampleRateXBitsPerSampleXChanngelsDivEight; // (Sample Rate * BitsPerSample * Channels) / 8
	analyzeHeaderDS >> bitsPerSampleXChannelsDivEightPointOne; // (BitsPerSample * Channels) / 8.1
	analyzeHeaderDS >> bitsPerSample; // Bits per sample

	// Print the header
	qDebug() << "WAV File Header read:";
	qDebug() << "File Type: " << QString::fromUtf8(fileType);
	qDebug() << "File Size: " << fileSize;
	qDebug() << "WAV Marker: " << QString::fromUtf8(waveName);
	qDebug() << "Format Name: " << QString::fromUtf8(fmtName);
	qDebug() << "Format Length: " << fmtLength;
	qDebug() << "Format Type: " << fmtType;
	qDebug() << "Number of Channels: " << numberOfChannels;
	qDebug() << "Sample Rate: " << sampleRate;
	qDebug() << "Sample Rate * Bits/Sample * Channels / 8: " << sampleRateXBitsPerSampleXChanngelsDivEight;
	qDebug() << "Bits per Sample * Channels / 8.1: " << bitsPerSampleXChannelsDivEightPointOne;
	qDebug() << "Bits per Sample: " << bitsPerSample;
	qDebug() << "Data Header: " << QString::fromUtf8(dataHeader);
	qDebug() << "Data Size: " << dataSize;

	QAudioFormat res;
	res.setChannelCount(numberOfChannels);
	res.setSampleRate(sampleRate);
	res.setSampleSize(bitsPerSample);

	return res;
}

} /* namespace cb */
