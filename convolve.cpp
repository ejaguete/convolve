#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <vector>
using std::fstream;
using std::cout;
using std::endl;
using std::vector;

typedef struct RIFF_CHUNK {
	// RIFF chunk descriptor
	uint8_t 	RIFF[4];						// RIFF header
	uint32_t	ChunkSize;						// full chunk size
	uint8_t		WAVE[4];						// WAVE header
} riff_chk;

typedef struct FMT_CHUNK {
	// fmt subchunk
	uint8_t		fmt[4];							// FMT header
	uint32_t	Subchunk1Size;					// size of fmt chunk
	uint16_t	AudioFormat;					// audio format
	uint16_t	NumChannels;					// number of channels
	uint32_t	SamplesPerSec;					// sampling frequency (Hz)
	uint32_t	BytesPerSec;					// bytes per second
	uint16_t	blockAlign;						// 2=16bit mono, 4 = 16bit stereo
	uint16_t	bitsPerSample;					// number of bits per sample
} fmt_chk;

typedef struct DATA_CHUNK {
	uint8_t		Subchunk2ID[4];					// "data" string
	uint32_t	Subchunk2Size;					// sampled data length
} data_chk;

typedef struct WAV_HEADER {
	riff_chk riff;
	fmt_chk fmt;
	data_chk data;
} wav_hdr;


int getFileSize(FILE *input);
void print_meta(int filelength, wav_hdr header);
void parseWavFile(const char wavFile[], vector<float> signal, bool doScaling);
void convolve(float x[], int N, float h[], int M, float y[], int P);

int main(int argc , char *argv[]) {

	if(argc<4) {
		printf("Missing arguments.\n");
		return 1;
	}	else {
		char *inputfile = argv[1];
		char *irfile = argv[2];
		char *outputfile = argv[3];
		
		printf("input file: %s\nIR file: %s\noutput file: %s\n",inputfile, irfile, outputfile);
		
		float *input[], float *ir[];
		input = parseWavFile(inputfile, input, true);
		
		return 0;
	}
	
}

int getFileSize(FILE *input) {
	int fileSize = 0;
	fseek(input, 0, SEEK_END);
	fileSize = ftell(input);
	fseek(input, 0, SEEK_SET);
	return fileSize;
}

void print_meta(int filelength, wav_hdr header) {
	// print wav file info
	cout << "RIFF header: 			\"" << header.riff.RIFF[0] << header.riff.RIFF[1] << header.riff.RIFF[2] << header.riff.RIFF[3] << "\"" << endl;
	cout << "chunk size: 			" << header.riff.ChunkSize << endl;
	cout << "WAVE header: 			\"" << header.riff.WAVE[0] << header.riff.WAVE[1] << header.riff.WAVE[2] << header.riff.WAVE[3] << "\"" << endl;
	
	cout << "fmt: 				\"" << header.fmt.fmt[0] << header.fmt.fmt[1] << header.fmt.fmt[2] << header.fmt.fmt[3] << "\"" << endl;
	cout << "subchunk1 size: 		" << header.fmt.Subchunk1Size << endl;
	// sampling rate
	cout << "sampling rate: 			" << header.fmt.SamplesPerSec << endl;
	cout << "number of bits used: 		" << header.fmt.bitsPerSample << endl;
	cout << "number of channels: 		" << header.fmt.NumChannels << endl;
	cout << "number of bytes/second: 	" << header.fmt.BytesPerSec << endl;
	cout << "audio format: 			" << header.fmt.AudioFormat << endl;
	cout << "block align: 			" << header.fmt.blockAlign << endl;
	
	cout << "data string: 			\"" << header.data.Subchunk2ID[0] << header.data.Subchunk2ID[1] << header.data.Subchunk2ID[2] << header.data.Subchunk2ID[3] << "\"" <<endl;
	cout << "data length: 			" << header.data.Subchunk2Size << endl;
}

void parseWavFile(const char wavFile[], vector<float> signal, bool doScaling) {
	
	FILE * wav;
	wav = fopen(wavFile, "r");
	if(wav == NULL) {
		fprintf(stderr, "Error opening files: %s\n\n", wavFile);
	} else {
		int wavlength = 0;
		wavlength = getFileSize(wav);
		if(wavlength>0) {
			wav_hdr header;
			int riffSize = sizeof(riff_chk);
			int fmtSize = sizeof(fmt_chk);
			int dataSize = sizeof(data_chk);
			
			// parse input file
			fread(&header.riff, 1, riffSize, wav);
			fread(&header.fmt, 1, fmtSize, wav);
			if(header.fmt.Subchunk1Size>16) {
				fread(&header.fmt, 1, header.fmt.Subchunk1Size-16, wav);
			}
			fread(&header.data, 1, dataSize, wav);
			
			cout << "---------\nwav file\n---------" << endl;
			print_meta(wavlength, header);
			
			uint16_t sampleSize = header.fmt.bitsPerSample;
			uint64_t numSamples = header.data.Subchunk2Size/sampleSize/8;		// how many samples in wav file
			int16_t BUFFER_LEN = numSamples;
			
			if(doScaling) {
				for(int i=0; i<numSamples; i++) {
					fread(&signal[i], 1, sampleSize, wav);
					signal[i] /= (float) 32768.0;
				}
			} else {
				for(int i=0; i<numSamples; i++) {
					fread(&signal[i], 1, sampleSize, wav);
				}
			}
		}
	}
}

void convolve(float x[], int N, float h[], int M, float y[], int P) {
	int n,m;
	if(P!=(N+M-1)) {
		cout << "output signal vector is wrong size\n" << endl;
		cout << "is " << P << ", but should be " << N+M-1 << endl; 
		cout << "aborting convolution" << endl;
		return;
	}
	
	for(n=0; n<P; n++) {
		y[n] = 0.0;
	}
	
	for(n=0; n<N; n++) {
		for(m=0; n<M; m++) {
			y[n+m] += x[n] * h[m];
		}
	}
}
