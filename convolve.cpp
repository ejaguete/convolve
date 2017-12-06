#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
using std::fstream;
using std::string;
using std::cout;
using std::endl;

typedef struct WAV_HEADER
{
	// RIFF chunk descriptor
	uint8_t 		RIFF[4];					// RIFF header
	uint32_t	ChunkSize;			// RIFF chunk size
	uint8_t		WAVE[4];				// WAVE header
	// fmt subchunk
	uint8_t		fmt[2];					// FMT header
	uint32_t	Subchunk1Size;	// size of fmt chunk
	uint16_t	AudioFormat;		// audio format
	uint16_t	NumChannels;		// number of channels
	uint32_t	SamplesPerSec;	// sampling frequency (Hz)
	uint32_t	BytesPerSec;		// bytes per second
	uint16_t	blockAlign;				// 2=16bit mono, 4 = 16bit stereo
	uint16_t	bitsPerSample;		// number of bits per sample
	uint16_t 	extra;						// in case chunk size is more than 16
	// data subchunk
	uint8_t		Subchunk2ID[4];	// "data" string
	uint32_t	Subchunk2Size;	// sampled data length
} wav_hdr;

int getFileSize(FILE *input);

int main(int argc , char *argv[]) {

	if(argc<4) {
		printf("Missing arguments.\n");
		return 1;
	}	else {
		char *inputfile = argv[1];
		char *irfile = argv[2];
		char *outputfile = argv[3];
		
		wav_hdr wavHeader;
		int headerSize = sizeof(wav_hdr), filelength = 0;
		
		printf("input file: %s\nIR file: %s\noutput file: %s\n",inputfile, irfile, outputfile);
		
		FILE * wavFile;
		wavFile = fopen(inputfile, "r");
		if(wavFile == NULL) {
			fprintf(stderr, "Error opening file: %s\n", inputfile);
			return 1;
		} else {
			// parse file
			
			// read header
			size_t bytesRead = fread(&wavHeader, 1, headerSize, wavFile);
			printf("header read %d bytes\n", bytesRead);
			
			if(bytesRead>0) {
				
				// read data
				uint16_t bytespersample = wavHeader.bitsPerSample/8;
				uint64_t numSamples = 
				wavHeader.ChunkSize/bytespersample;		// how many samples in wav file
				static const uint16_t BUFFER_SIZE = 4096;
				int8_t *buffer = new int8_t[BUFFER_SIZE];
				
				// read data
				while((bytesRead=fread(buffer, sizeof buffer[0], 
				BUFFER_SIZE / (sizeof buffer[0]), wavFile)) > 0) {
					// do smth with data
					//printf("read %d bytes", bytesRead); 
				}
				
				delete [] buffer;
				buffer = nullptr;
				filelength = getFileSize(wavFile);
				
				cout << "file is: 			" << filelength << " bytes" << endl;
				cout << "RIFF header: 			\"" << wavHeader.RIFF[0] << wavHeader.RIFF[1] << wavHeader.RIFF[2] << wavHeader.RIFF[3] << "\"" << endl;
				cout << "WAVE header: 			\"" << wavHeader.WAVE[0] << wavHeader.WAVE[1] << wavHeader.WAVE[2] << wavHeader.WAVE[3] << "\"" << endl;
				cout << "fmt: 				\"" << wavHeader.fmt[0] << wavHeader.fmt[1] << wavHeader.fmt[2] << wavHeader.fmt[3] << "\"" << endl;
				cout << "subchunk1 size: 		" << wavHeader.Subchunk1Size << endl;
				cout << "data size: 			" << wavHeader.ChunkSize << endl;
				
				// sampling rate
				cout << "sampling rate: 			" << wavHeader.SamplesPerSec << endl;
				cout << "number of bits used: 		" << wavHeader.bitsPerSample << endl;
				cout << "number of channels: 		" << wavHeader.NumChannels << endl;
				cout << "number of bytes/second: 	" << wavHeader.BytesPerSec << endl;
				cout << "data length: 			" << wavHeader.Subchunk2Size << endl;
				cout << "audio format: 			" << wavHeader.AudioFormat << endl;
				cout << "block align: 			" << wavHeader.blockAlign << endl;
				cout << "data string: 			\"" << wavHeader.Subchunk2ID[0] << wavHeader.Subchunk2ID[1] << wavHeader.Subchunk2ID[2] << wavHeader.Subchunk2ID[3] << "\"" <<endl;
			}
			fclose(wavFile);
			return 0;
		}
	}
}

int getFileSize(FILE *input) {
	int fileSize = 0;
	fseek(input, 0, SEEK_END);
	fileSize = ftell(input);
	fseek(input, 0, SEEK_SET);
	return fileSize;
}
