#pragma once
#include <stdint.h>
#include <string>
#include <stdio.h>

namespace flv{

#define  DEFAULT_BUFFER_LEN 1024 * 1024
#define  DEFAULT_BUFFER_INCR_FACTOR 2


#define  SCRIPT_DATA_PREVIOUS_LEN 0
#define  SCRIPT_DATA_AMF2_PROPERTIES_NUM 12

#define  TAG_HEADER_SIZE 11

	// audio basic define
#define FLV_AUDIO_SAMPLESSIZE_OFFSET 1
#define FLV_AUDIO_SAMPLERATE_OFFSET  2
#define FLV_AUDIO_CODECID_OFFSET     4

#define FLV_VIDEO_FRAMETYPE_OFFSET   4

	/* bitmasks to isolate specific values */
#define FLV_AUDIO_CHANNEL_MASK    0x01
#define FLV_AUDIO_SAMPLESIZE_MASK 0x02
#define FLV_AUDIO_SAMPLERATE_MASK 0x0c
#define FLV_AUDIO_CODECID_MASK    0xf0

#define FLV_VIDEO_CODECID_MASK    0x0f
#define FLV_VIDEO_FRAMETYPE_MASK  0xf0

#define AMF_END_OF_OBJECT         0x09

#define AUDIO_SAMPLE_RATE	 8000
#define AUDIO_SAMPLE_SIZE	 16

#define AUDIO_SAMPLE_RATE_8K 0
#define AUDIO_SAMPLE_SIZE_16 1
#define AUDIO_MONO			 0
#define AUDIO_STEREO		 1

#define AUDIO_CODEC_PCMA	 7
#define AUDIO_CODEC_PCMU	 8
#define AUDIO_CODEC_AAC		 10

	enum
	{
		FLV_HEADER_FLAG_HASVIDEO = 1,
		FLV_HEADER_FLAG_HASAUDIO = 4,
	};

	static const char *pWidth = "width";
	static const char *pHeight = "height";
	static const char *pDuration = "duration";
	static const char *pFramerate = "framerate";
	static const char *pVCodecId = "videocodecid";
	static const char *pFilesize = "filesize";
	static const char *pVDataRate = "videodatarate";
	static const char *pAcodecID = "audiocodecid";
	static const char *pASampleRate = "audiosamplerate";
	static const char *pASampleSize = "audiosamplesize";
	static const char *pStereo = "stereo";
	static const char *pADataRate = "audiodatarate";
	static const char *pAmfEnd = "";

	/* amf properties value type*/
	enum script_data_type {
		Number = 0,
		Boolean,
		String,
		Object,
		MovieClip,
		Null,
		Undefined,
		Reference,
		EcmaArray,
		ObjectEndMarker,
		StringArray,
		Date,
		LongString,
	};

	/* amf type */
	enum tag_type
	{
		FLV_TAG_TYPE_AUDIO = 0x08,
		FLV_TAG_TYPE_VIDEO = 0x09,
		FLV_TAG_TYPE_META = 0x12,
	};

	/*video codec id*/
	enum
	{
		FLV_CODECID_H264 = 7,
	};

	/*
	
	video frame type
	 4 bit: key or inner fram
	 4 bit: encode type, default 7(AVC)
	
	*/
	enum
	{
		FLV_FRAME_KEY = 1 << FLV_VIDEO_FRAMETYPE_OFFSET | 7,
		FLV_FRAME_INTER = 2 << FLV_VIDEO_FRAMETYPE_OFFSET | 7,
	};


	/* audio codec id*/
	enum
	{
		FLV_CODECID_MP3 = 2 << FLV_AUDIO_CODECID_OFFSET,
		FLV_CODECID_PCMA = 7 << FLV_AUDIO_CODECID_OFFSET,
		FLV_CODECID_PCMU = 8 << FLV_AUDIO_CODECID_OFFSET,
		FLV_CODECID_AAC = 10 << FLV_AUDIO_CODECID_OFFSET,
	};

	enum head_flags
	{
		FLAG_AUDIO = 0x04,
		FLAG_VIDEO = 0x01,
	};

#pragma pack(push, 1)
	typedef struct flvheader_tag
	{
		flvheader_tag(){
			flv_signature[0] = 0x46;
			flv_signature[1] = 0x4c;
			flv_signature[2] = 0x56;
		}
		/*
		signature, default "FLV"--0x46, 0x4c, 0x56
		*/
		uint8_t  flv_signature[3];
		/*
		version, default 0x01
		*/
		uint8_t  flv_version = 0x01;
		/* LB-- 0    1   2   3   4    5      6      7  --HB
		        0    0   0   0   0   Audio   0     Video
		*/
		uint8_t  flv_flags;
		/*
		header size
		*/
		uint32_t flv_headersize = sizeof(struct flvheader_tag);
	} flvheader;

	typedef struct tagheader_tag
	{
		/*
		audio:0x08
		video:0x09
		script:0x12
		*/
		uint8_t tagtype;
		/*
		data size;
		*/
		uint8_t datasize[3];
		/*
		timestamp;
		*/
		uint8_t timestamp[3];
		/*
		timestamp_ex
		*/
		uint8_t timestamp_ex;
		/*
		stream id;
		*/
		uint8_t streamid[3];
	} tagheader;


	typedef struct amf1_tag
	{
		/*
		amf type:
		string: 0x02
		*/
		uint8_t amftype = 0x02;
		/*
		amf string len
		*/
		uint16_t amflen = 0x000A;
		/*
		amf string
		*/
		uint8_t amfstring[0];
	}amf1;

	typedef struct amf2_tag
	{
		uint8_t amftype;
		uint32_t amflen;
		/*
		array format:

		      2 bytes          n bytes        1 bytes       n bytes
		 |----field 1----|-----field2----|----field3-----|----field4---|

		 field 1: properties name length
		 field 2: properties name
		 field 3: properties value type
		 field 4: properties value(fixed length)
		*/
		uint8_t amfproperties[0];
	}amf2;

	typedef struct audiodata_tag
	{
		uint8_t audio_metadata;
		uint8_t audio_data[0];
	}audiodata;

	typedef struct videodata_tag
	{
		uint8_t video_metadata;
		uint8_t video_data[0];
	} video_data;


	typedef union{
		double dbl;
		uint64_t ui64;
	} dlui;
	
#pragma pack(pop)


	class FlvWriter
	{
	public:
		FlvWriter() :m_flvData(nullptr), m_flvDataLen(0), m_file(nullptr), 
			m_headFlags(0), m_bHaveSetSPSPPS(false){};
		FlvWriter(const FlvWriter &other) = delete;
		FlvWriter & operator=(const FlvWriter &other) = delete;
		~FlvWriter(){};

		bool CreateWriter(std::string & flvName);
		void CloseWriter();
		/* head*/
		void setHeadFlag(uint8_t headFlags);
		/* audio */
		void setAudioCodec(uint32_t codecType);

		void setSoundFormat(uint16_t SoundFormat);
		void setSoundRate(uint16_t SampleRate);
		void setSoundSize(uint16_t SampleSize);
		void setSoundType(uint16_t AudioType);

		uint32_t getAudioCode();
		uint16_t getAudioSampleRate();
		uint16_t getAudioSampleSize();
		uint16_t getAudioSampleChannels();
		/* video */
		void setVideoCodec(uint32_t codecType);
		uint32_t getVideoCodec();

		/*flv script param*/
		void setScriptParam(uint16_t uiWidth, uint16_t uiHeight, double fps);

		inline void setLastTagSize(uint64_t value)
		{
			m_lastTagSize = value;
		};

		inline uint64_t getLastTagSize()
		{
			return m_lastTagSize;
		};
		
		/* Write flv*/
		bool WriteFlvHead(bool flush = false);
		bool WriteFlvScript(bool flush = false);
		bool AppendSPSPPS(const char *psps, uint64_t spslen, const char *ppps, uint64_t pppslen, bool flush = false);
		bool AppendVideoFrame(const char *pdata, uint64_t len, bool flush = false);
		bool AppendAudioFrame(const char *pdata, uint64_t len, bool flush = false);

	private:

		void appendbyte(uint8_t value);
		void appendshort(uint16_t value);
		void appendint24(uint32_t value);
		void rewriteint24(uint32_t value, uint64_t pos);
		void appendint(uint32_t value);
		void appendlong(uint64_t value);
		void appendDouble(double dbvalue);
		void appendString(const char *pdata, uint32_t strlen);

		void append_amf_Double(double dbvalue);
		void append_amf_String(const char *string, uint16_t strlen);

		bool flushData();
		uint64_t getCurBufOffset();


		/*utils*/
		bool reAllocMem(uint64_t needSize);
		bool checkAndreAllcoMem(uint64_t needSize);
		/* */
		inline bool isLittleEndian()
		{
			static union{ uint8_t i8[2]; uint16_t i16; } a;
			a.i16 = 0x0102;
			if (a.i8[0] == 0x01 && a.i8[1] == 0x02)
			{
				return false;
			}
			return true;
		};

		inline uint16_t reverse16(uint16_t value)
		{
			return ((value >> 8) & 0xff) | ((value << 8) & 0xff00);
		};

		inline uint32_t reverse32(uint32_t value)
		{
			 return (reverse16(value & 0xffff) << 16) | reverse16((value >> 16) & 0xffff);
		};

		inline uint64_t reverse64(uint64_t value)
		{
			return (reverse32(value & 0xffffffff) << 32) | (reverse32((value >> 32) & 0xffffffff));
		};

		inline uint16_t toBigEndian(uint16_t value)
		{
			if (isLittleEndian())
			{
				return reverse16(value);
			}
			return value;
		};

		inline uint16_t toLittleEndian(uint16_t value)
		{
			if (isLittleEndian())
			{
				return value;
			}
			return reverse16(value);
		};

		inline uint32_t toBigEndian(uint32_t value)
		{
			if (isLittleEndian())
			{
				return reverse32(value);
			}

			return value;
		};

		inline uint32_t toLittleEndian(uint32_t value){
			if (isLittleEndian())
			{
				return value;
			}
			return reverse32(value);
		}

		inline uint64_t toBigEndian(uint64_t value)
		{
			if (isLittleEndian())
			{
				return reverse64(value);
			}
			return value;
		};

		inline uint64_t toLittleEndian(uint64_t value)
		{
			if (isLittleEndian())
			{
				return value;
			}
			return reverse64(value);
		};

		inline uint64_t dl2ui64(double value){
			static dlui var;
			var.dbl = value;
			return var.ui64;
		}

		uint32_t m_AudioCodecType;

		uint64_t m_cts;
		uint64_t m_dts;
		uint64_t m_step;
		/* write SPS/PPS just once*/
		bool m_bHaveSetSPSPPS;

		uint8_t m_headFlags;

		bool m_bHaveWriteAACSeqHeader;

		uint32_t m_VideoCodecType;

		uint64_t m_lastTagSize;

		/*flv properties*/
		uint16_t m_Height;
		uint16_t m_Width;
		double m_FrameRate;

		uint8_t m_SoundFormat;
		uint8_t m_SoundRate;
		uint8_t m_SoundSize;
		uint8_t m_SoundType;


		uint8_t* m_flvData;
		uint64_t m_flvDataLen;	
		uint64_t m_flvWriterOffset;
		FILE *m_file;
	};  
}



