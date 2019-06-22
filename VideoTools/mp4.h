#pragma once
#include <stdint.h>
#include <string>
#include <stdio.h>
#include "util.h"
#include <vector>


// Note: 
/************************************************************************/
/* moov： save video info and index
   mdat:  save video frames , if there is no mdata structure, then video 
   frames are reference from other files                                 */
/************************************************************************/


namespace mp4{

#define  DEFAULT_BUFFER_LEN 1024 * 1024
#define  DEFAULT_BUFFER_INCR_FACTOR 2

#define  DEFAULT_VERSION 0x66
#define  DEFAULT_BRAND_LEN 4

/* key frame nums of a chunk*/
#define  DEFAULT_CHUNK_SAMPLE_NUM 128

	static const char *pisom = "isom";
	static const char *piso2 = "iso2";
	static const char *pavc1 = "avc1";
	static const char *pmp41 = "mp41";

	static const char *pftyp = "ftyp";
	static const char *p = "";
	

/********************************************************************************************
**		字段名称			|	长度(bytes)	|		有关描述
--------------------------------------------------------------------------------------------
**		boxsize				|	4			|		box的长度
**		boxtype				|	4			|		box的类型
********************************************************************************************/

	/*
	common box header for all box
	*/
	typedef struct box_header_tag
	{
		uint32_t boxSize;
		uint8_t boxType[4];
	} box_header;

	/*
	first class box:
	   ftyp:   file type box
	   moov:   metadata container, control and index info
	   mdat:   mediadata container, real media data
	   free:   for data align
	*/
	typedef struct ftyp_box_tag
	{
		box_header stHead;
		uint8_t majorBrand[4];
		uint32_t minorVersion;
		uint32_t compatibleBrands[4];
	} ftyp_box;

	typedef struct mdat_box_tag
	{
		box_header stHead;
	} mdat_box;

	typedef struct free_box_tag
	{
		box_header stHead;
	}free_box;

	typedef struct moov_box_tag
	{
		box_header stHead;
	} moov_box;

	/*
	Second class box: (sub box of moov)
	     mvhd: movie header
		 trak: track or stream container,could one or many
		       everyone is a box for different track of video or audio data stream
		 udat: user defined data box
	*/
	typedef struct mvhd_box_tag
	{
		box_header stHead;
		uint8_t  boxVersion;
		uint8_t  boxFlags[3];
		uint32_t boxCreateTime;
		uint32_t boxModifyTime;
		uint32_t boxTimeScale;
		uint32_t boxDuration;
		uint32_t boxRate;
		uint16_t boxVolume;
		uint8_t  boxReserved[10];
		uint8_t  boxMatrix[36];
		uint8_t  boxPreDefined[24];
		uint32_t boxNextTrackID;
	}mvhd_box;

	typedef struct trak_box_tag
	{
		box_header stHead;
	}trak_box;


	//user defined data
	typedef struct udat_box_tag
	{
		box_header stHead;
	}udat_box;

	/************************************************************************/
	/* Third class box: (sub box of trak)
	       tkhd:   track header box
	       mdia:  track media info container
	*/
	/************************************************************************/
	typedef struct tkhd_box_tag
	{
		box_header stHead;
		uint8_t  boxVersion;
		uint8_t  boxFlags[3];
		uint32_t boxCreateTime;
		uint32_t boxModificationTime;
		uint32_t boxTrackID;
		uint32_t boxReserved;
		uint32_t boxDuration;
		uint64_t boxReserved;
		uint16_t boxLayer; // video layer small on top
		uint16_t boxAltGroup; //
		uint16_t boxVolume;
		uint16_t boxReserved;
		uint8_t  boxMatrix[36];
		uint32_t boxWidth;
		uint32_t boxHeigt;
	}tkhd_box;

	typedef struct media_box_tag
	{
		box_header stHead;
	}mdia_box;

	/************************************************************************/
	/* Fourth class box:(sub box of  mdia)
	           mdhd:  header of mdia box
			   hdlr:  handler,meaning track type 
			   minf: media info container
	*/
	/************************************************************************/
	typedef struct mdhd_box_tag
	{
		box_header stHead;
		uint8_t  boxVersion;
		uint8_t  boxFlags[3];
		uint32_t boxCreateTime;
		uint32_t boxModificationTime;
		uint32_t boxTimeScale;
		uint32_t boxDuration;
		uint16_t boxLanguage;
		uint16_t boxPreDefined;
	}mdhd_box;

	typedef struct hdlr_box_tag
	{
		box_header stHead;
		uint8_t  boxVersion;
		uint8_t  boxFlags[3];
		uint32_t boxPreDefined;
		uint32_t boxHandlerType;
		uint8_t  boxReserved[12];
		//uint8_t pName[0];     //track type name, string end with '\0'
	}hdlr_box;

	typedef struct minf_box_tag
	{
		box_header stHead;
	}minf_box;

	/************************************************************************/
	/* Fifth class box:(sub box of minf)
	     head: vmhd smhd hmhd nmhd
		 dinf: dref: url urn
		 stbl: sample table box
	*/
	/************************************************************************/
	typedef struct vmhd_box_tag
	{
		box_header stHead;
		uint8_t    boxVersion;
		uint8_t    boxFlags[3];
		uint32_t   boxGraphMode;
		uint8_t    boxOpColor;
	}vmhd_box;

	typedef struct stbl_box_tag
	{
		box_header stHead;
	}stbl_box;

	/************************************************************************/
	/*Sixth class box:(sub class of stbl)
	      stsd: 
		  stts:
		  stsz:
		  stz2:
		  stss:
		  stco:
		  co64:
	*/
	/************************************************************************/
/************************************************************************************************************
**										stsd
**
-------------------------------------------------------------------------------------------------------------
**		字段名称			|	长度(bytes)	|		有关描述
-------------------------------------------------------------------------------------------------------------
**		boxsize				|	4			|		box的长度
**		boxtype				|	4			|		box的类型
**		version				|	1			|		box版本0或1 一般为0 (以下字节数均按version=0)
**		flags				|	3			|
**		entry_count			|	4			|
************************************************************************************************************/
	typedef struct box_stsd_tag
	{
		box_header stHead;
		uint8_t  boxVersion;
		uint8_t  boxFlags[3];
		uint32_t boxEntryCount;
	} box_stsd;

	typedef struct box_sampledata_tag
	{
		uint32_t sampleCount;
		uint32_t sampleDelta;
	}box_sampledata;

	typedef struct box_stts_tag
	{
		box_header stHead;
		uint32_t boxEntryCount;
		box_sampledata pSampleDatas[0];
	}box_stts;

	typedef struct stscData_tag
	{
		uint32_t firstChunk;
		uint32_t samplePerChunk;
		uint32_t sampleDesIndex;
	} stsc_Data;

	typedef struct box_stsc_tag
	{
		box_header stHead;
		uint8_t  boxVersion;
		uint8_t  boxFlags[3];
		uint32_t boxEntryCount;
		//stsc_Data pStscData[0];
	}box_stsc;

	typedef struct box_stsz_tag
	{
		box_header stHead;
		uint32_t boxVersion;
		uint32_t boxFlags[3];
		uint32_t sampleSize;
		uint32_t sampleCount;
		//uint32_t sampleSize[0];
	} box_stsz;

	// I frame sequence
	typedef struct box_stss_tag
	{
		box_header stHead;
		uint32_t boxEntryCount;
		//uint32_t pSampleNum[0];
	} box_stss;

	typedef struct box_stco_tag
	{
		box_header stHead;
	}box_stco;


	class Mp4Writer
	{
	public:
		Mp4Writer(): m_mp4Data(nullptr), m_mp4DataLen(0)
			, m_mp4WriterOffset(0), m_flushLen(0){};
		// forbide copy construct
		Mp4Writer(const Mp4Writer& other) = delete;
		~Mp4Writer(){};

		bool CreateWriter(std::string & mp4Name);
		void CloseWriter();

		bool WriteFileTypeBox(bool bFlush = false);
		bool WriteKeyFrame(const char *pbuf, uint32_t len, bool bFlush = false);
		bool WriteNormalFrame(const char *pbuf, uint32_t len, bool bFlush = false);
		bool WriteMoovBox(bool bFulsh = false);

		bool AppendString(const char *pbuf, uint32_t len);
	
	private:
		bool flushData();
		uint64_t getCurBufOffset();

		/*utils*/
		bool reAllocMem(uint64_t needSize);
		bool checkAndreAllcoMem(uint64_t needSize);

	private:

		//sixth class box
		// key frame list and count
		box_stss m_stssBox;
		std::vector<uint32_t> m_vKeySampleCount;

		//sample to chunk
		box_stsc m_stscBox;
		std::vector<stsc_Data> m_vStsc;
		uint32_t m_samplePerChunk;

		//sample size
		box_stsz m_stszBox;
		std::vector<uint32_t> m_vSampleSize;

		//chunk offset
		box_stco m_stcoBox;
		std::vector<uint32_t> m_vChunkOffset;

		//fifth class box
		stbl_box m_stblBox;

		//fourth class box
		minf_box m_minfBox;
		hdlr_box m_hdlrBox;
		mdhd_box m_mdhdBox;
		//third class box
		mdia_box m_mdiaBox;
		tkhd_box m_tkhdBox;

		//second class box
		mvhd_box m_mvhdVideoBox;
		trak_box m_trakVideoBox;

		//first class box
		ftyp_box m_ftypBox;
		moov_box m_moovBox;
		mdat_box m_mdatBox;


		// file memory
		uint8_t* m_mp4Data;
		uint64_t m_mp4DataLen;
		uint64_t m_mp4WriterOffset;
		uint64_t m_flushLen;
		FILE *m_file;

		//moov box memory

	};

};