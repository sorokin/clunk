#ifndef __SAMPLE_CLUNK_OGG_STREAM_H__
#define __SAMPLE_CLUNK_OGG_STREAM_H__

//This example is the public domain

#include <string>
#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisenc.h>
#include <vorbis/vorbisfile.h>
#include "clunk/stream.h"
#include <stdio.h>

namespace clunk {
	class Sample;
	class Buffer;
}

class OggStream : public clunk::Stream {
public: 
	OggStream(const std::string &fname);
	void rewind();
	bool read(clunk::Buffer &data, unsigned hint);
	~OggStream();

private: 
	FILE * _file;
	OggVorbis_File _ogg_stream;
	vorbis_info * _vorbis_info;
};

#endif

