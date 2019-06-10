#include "stdafx.h"
#include "Flv.h"

void flv::FlvWriter::setHeadFlag(uint8_t headFlags)
{
	m_headFlags |= headFlags;
}

bool flv::FlvWriter::CreateWriter(std::string & flvName)
{
	m_file = fopen(flvName.c_str(), "wb");
	if (m_file == nullptr)
	{
		printf("open file %s failed\n", flvName.c_str());
		return false;
	}

	if (m_flvData == nullptr)
	{
		m_flvDataLen = sizeof(uint8_t) * DEFAULT_BUFFER_LEN;
		m_flvData = (uint8_t *)malloc(m_flvDataLen);
		
		if (m_flvData == nullptr)
		{
			m_flvData = nullptr;
			m_flvDataLen = 0;
			printf("malloc buffer failed!\n");
			return false;
		}
		m_flvWriterOffset = 0;
	}


}

void flv::FlvWriter::setAudioCodec(uint32_t codecType)
{
	m_AudioCodecType = codecType;
}


uint32_t flv::FlvWriter::getAudioCode()
{
	return m_AudioCodecType;
}


void flv::FlvWriter::setVideoCodec(uint32_t codecType)
{
	m_VideoCodecType = codecType;
}

uint32_t flv::FlvWriter::getVideoCodec()
{
	return m_VideoCodecType;
}

bool flv::FlvWriter::WriteFlvHead(bool flush /*= false*/)
{
	flvheader flvHead;
	flvHead.flv_flags = m_headFlags;
	flvHead.flv_headersize = toBigEndian(flvHead.flv_headersize);
	

	if (m_flvWriterOffset + sizeof(flvHead) > m_flvDataLen)
	{
		if (!reAllocMem(sizeof(flvHead)))
		{
			printf("Writer head failed, get memory failed!");
			return false;
		}
	}
	memcpy(m_flvData + m_flvWriterOffset, &flvHead, sizeof(flvHead));
	m_flvWriterOffset += sizeof(flvHead);

	if (flush)
	{
		return flushData();
	}
	return true;
}

bool flv::FlvWriter::reAllocMem(uint64_t needSize)
{
	printf("re-alloc memory\n");
	m_flvDataLen = (m_flvDataLen > needSize ? m_flvDataLen:needSize) * DEFAULT_BUFFER_INCR_FACTOR;
	/* increase memory just after origin memory*/
	uint8_t *newMem = (uint8_t *)realloc(m_flvData, m_flvDataLen);
	if (newMem == nullptr)
	{

		/* try a new memory block*/
		newMem = (uint8_t *)malloc(m_flvDataLen);
		if (newMem == nullptr)
		{
			printf("re-alloc memory failed\n");
			return false;
		}
		memcpy(newMem, m_flvData, m_flvWriterOffset);
		free(m_flvData);
		m_flvData = newMem;
		return true;
	}
	return true;
}

bool flv::FlvWriter::AppendVideoFrame(const char *pdata, uint64_t len, bool flush /*= false*/)
{
	/*
	* short: 00 00 01
	* long:  00 00 00 01
	*/
	static bool bShortHeaderIdentifer = false;
	static uint8_t skipHeaderLen = 4;
	if (pdata[2] == 1)
	{
		bShortHeaderIdentifer = true;
		skipHeaderLen = 3;
	}
	else
	{
		bShortHeaderIdentifer = false;
		skipHeaderLen = 4;
	}
	//previous tag length
	appendint(getLastTagSize());
	//tag type :script data
	appendbyte(flv::tag_type::FLV_TAG_TYPE_VIDEO);
	uint64_t dataSizePos = getCurBufOffset();
	//tag data size
	appendint24(0);  //re-calculate later
	//tag timestamp
	appendint24(m_cts);
	//tag timestampex
	appendbyte(m_cts >> 24);
	//tag stream id
	appendint24(0);
	uint64_t dataBufStart = getCurBufOffset();

	// video tag data :first byte, key frame or normal frame
	if ((bShortHeaderIdentifer && (pdata[3] & 0x0f) == 5) || (!bShortHeaderIdentifer && ((pdata[4] & 0x0f) == 5)))
	{
		appendbyte(flv::FLV_FRAME_KEY);
	}
	else
	{
		appendbyte(flv::FLV_FRAME_INTER);
	}

	uint64_t offset = m_cts - m_dts;
	m_dts = m_cts;
	// AVC NALU
	appendbyte(1);
	// composition time offset
	appendint24(0);
	// skip  NALU header : 0x00, 0x00, 0x00, 0x01
	appendint(len - skipHeaderLen);

	appendString(pdata + skipHeaderLen, len - skipHeaderLen);

	uint64_t dataBufEnd = m_flvWriterOffset;

	//calculate tag data size and rewrite data size field
	rewriteint24(dataBufEnd - dataBufStart, dataSizePos);

	setLastTagSize(dataBufEnd - dataBufStart + TAG_HEADER_SIZE);

	m_cts += m_step;
	if (flush)
	{
		return flushData();
	}
	return true;
}

bool flv::FlvWriter::AppendAudioFrame(const char *pdata, uint64_t len, bool flush /*= false*/)
{
	//previous tag length
	appendint(getLastTagSize());
	//tag type :script data
	appendbyte(flv::tag_type::FLV_TAG_TYPE_AUDIO);
	uint64_t dataSizePos = getCurBufOffset();
	//tag data size
	appendint24(0);  //re-calculate later
	//tag timestamp
	appendint24(m_cts);
	//tag timestampex
	appendbyte(m_cts >> 24);
	//tag stream id
	appendint24(0);
	uint64_t dataBufStart = getCurBufOffset();

	uint8_t audio_header = m_SoundType & 0x01;
	audio_header |= (m_SoundSize << 1) & 0x02;
	audio_header |= (m_SoundRate << 2) & 0x0c;
	audio_header |= (m_SoundFormat << 4) & 0xf0;

	appendbyte(audio_header);

	if (m_SoundFormat == 10) //aac
	{
		if (m_bHaveWriteAACSeqHeader == false)
		{
			appendbyte(0);
			m_bHaveWriteAACSeqHeader = true;
		}
		else
		{
			appendbyte(1);
		}
	}

	appendString(pdata, len);

	uint64_t dataBufEnd = getCurBufOffset();
	uint64_t length = dataBufEnd - dataBufStart;
	rewriteint24(length, dataSizePos);
	setLastTagSize(length + TAG_HEADER_SIZE);
	return true;
}

bool flv::FlvWriter::WriteFlvScript(bool flush /*= false*/)
{
	//previous tag length
	appendint(SCRIPT_DATA_PREVIOUS_LEN);
	//tag type :script data
	appendbyte(flv::tag_type::FLV_TAG_TYPE_META);
	uint64_t dataSizePos = getCurBufOffset();
	//tag data size
	appendint24(0);
	//tag timestamp
	appendint24(0);
	//tag timestampex
	appendbyte(0);
	//tag stream id
	appendint24(0);
	uint64_t dataBufStart = getCurBufOffset();


	amf1 stuamf1; /*type, len, string*/
	amf2 stuamf2; /*type, arraylen array*/

	stuamf1.amflen = toBigEndian(stuamf1.amflen);

	if (!checkAndreAllcoMem(sizeof(amf1)))
	{
		printf("memory error, write flv script data failed\n");
		return false;
	}
	memcpy(m_flvData + m_flvWriterOffset, &stuamf1, sizeof(stuamf1));
	m_flvWriterOffset += sizeof(stuamf1);
	appendString("onMetaData", strlen("onMetaData"));

	stuamf2.amftype = flv::script_data_type::EcmaArray;
	stuamf2.amflen = SCRIPT_DATA_AMF2_PROPERTIES_NUM;
	stuamf2.amflen = toBigEndian(stuamf2.amflen);

	memcpy(m_flvData + m_flvWriterOffset, &stuamf2, sizeof(stuamf2));
	m_flvWriterOffset += sizeof(stuamf2);

	append_amf_String(flv::pWidth, strlen(flv::pWidth));
	append_amf_Double(m_Width);
	append_amf_String(flv::pHeight, strlen(flv::pHeight));
	append_amf_Double(m_Height);
	append_amf_String(flv::pFramerate, strlen(flv::pFramerate));
	append_amf_Double(m_FrameRate);
	append_amf_String(flv::pVCodecId, strlen(flv::pVCodecId));
	append_amf_Double(FLV_CODECID_H264);
	append_amf_String(flv::pDuration, strlen(flv::pDuration));
	//TODO: what is 1.64 is ?
	append_amf_Double(1.64);
	append_amf_String(flv::pFilesize, strlen(flv::pFilesize));
	//TODO: why file size zero?
	append_amf_Double(0);
	append_amf_String(flv::pVDataRate, strlen(flv::pVDataRate));
	append_amf_Double(384.00);
	append_amf_String(flv::pAcodecID, strlen(flv::pAcodecID));
	switch (m_SoundFormat)
	{
	    case 2:  append_amf_Double(flv::FLV_CODECID_MP3); break;
		case 10: append_amf_Double(flv::FLV_CODECID_AAC); break;
		case 7:  append_amf_Double(flv::FLV_CODECID_PCMA);break;
		case 8:  
		default:append_amf_Double(flv::FLV_CODECID_PCMU); break;
	}
	append_amf_String(flv::pASampleRate, strlen(flv::pASampleRate));
	switch (m_SoundRate)
	{
		case 1: append_amf_Double(11000); break;
		case 2: append_amf_Double(22000); break;
		case 3: append_amf_Double(44000); break;
		case 0:
		default:append_amf_Double(5500); break;
	}
	append_amf_String(flv::pASampleSize, strlen(flv::pASampleSize));
	append_amf_Double(m_SoundSize == 0 ? 8 : 16);
	append_amf_String(flv::pStereo, strlen(flv::pStereo));
	append_amf_Double(m_SoundType == 0 ? 0 : 1);
	append_amf_String(flv::pADataRate, strlen(flv::pADataRate));
	//TODO: what 0.00 is ?
	append_amf_Double(0.00);
	append_amf_String(flv::pAmfEnd, strlen(flv::pAmfEnd));
	append_amf_Double(AMF_END_OF_OBJECT);	
	uint64_t dataBufEnd = getCurBufOffset();

	//calculate tag data size and rewrite data size field
	rewriteint24(dataBufEnd - dataBufStart, dataSizePos);

	setLastTagSize(dataBufEnd - dataBufStart + TAG_HEADER_SIZE);

	if (flush)
	{
		flushData();
	}
	return true; 
}

void flv::FlvWriter::append_amf_Double(double dbvalue)
{
	appendbyte(flv::script_data_type::Number);
	appendDouble(dbvalue);
}

void flv::FlvWriter::append_amf_String(const char *string, uint16_t strlen)
{
	// for script data properties name use, no type
	//appendbyte(flv::script_data_type::String);
	appendshort(strlen);
	appendString(string, strlen);
}

void flv::FlvWriter::appendbyte( uint8_t value)
{
	if (!checkAndreAllcoMem(sizeof(uint8_t)))
	{
		printf("memory error, append byte failed\n");
		return;
	}

	m_flvData[m_flvWriterOffset++] = value;
}

bool flv::FlvWriter::checkAndreAllcoMem(uint64_t needSize)
{
	if (m_flvWriterOffset + needSize < m_flvDataLen)
	{
		return true;
	}
	return reAllocMem(needSize);
}

void flv::FlvWriter::appendshort(uint16_t value)
{
	if (!checkAndreAllcoMem(sizeof(uint16_t)))
	{
		printf("memory error, append uint16_t failed\n");
		return;
	}

	uint16_t var = toBigEndian(value);
	memcpy(m_flvData + m_flvWriterOffset, &var, sizeof(var));
	m_flvWriterOffset += sizeof(var);	
}

void flv::FlvWriter::appendint(uint32_t value)
{
	if (!checkAndreAllcoMem(sizeof(uint32_t)))
	{
		printf("memory error, append uint32_t failed\n");
		return;
	}

	uint32_t var = toBigEndian(value);
	memcpy(m_flvData + m_flvWriterOffset, &var, sizeof(var));
	m_flvWriterOffset += sizeof(var);
}

void flv::FlvWriter::appendlong(uint64_t value)
{
	if (!checkAndreAllcoMem(sizeof(uint64_t)))
	{
		printf("memory error, append uint64_t failed\n");
		return;
	}

	uint64_t var = toBigEndian(value);
	memcpy(m_flvData + m_flvWriterOffset, &var, sizeof(var));
	m_flvWriterOffset += sizeof(var);
}

void flv::FlvWriter::appendDouble(double dbvalue)
{
	appendlong(dl2ui64(dbvalue));
}

void flv::FlvWriter::appendString(const char *pdata, uint32_t strlen)
{
	if (!checkAndreAllcoMem(strlen))
	{
		printf("memory error, append uint64_t failed\n");
		return; 
	}

	memcpy(m_flvData + m_flvWriterOffset, pdata, strlen);
	m_flvWriterOffset += strlen;
}

void flv::FlvWriter::appendint24(uint32_t value)
{
	if (!checkAndreAllcoMem(3))
	{
		printf("memory error, append uint24_t failed\n");
		return;
	}

	uint32_t var = toBigEndian(value);
	/*skip high 8 bits*/
	memcpy(m_flvData + m_flvWriterOffset, (uint8_t *)&var + 1, 3);
	m_flvWriterOffset += 3;
}

void flv::FlvWriter::rewriteint24(uint32_t value, uint64_t pos)
{
	if (!checkAndreAllcoMem(3))
	{
		printf("memory error, append uint24_t failed\n");
		return;
	}

	uint32_t var = toBigEndian(value);
	/*skip high 8 bits*/
	memcpy(m_flvData + pos, (uint8_t *)&var + 1, 3);
}

uint64_t flv::FlvWriter::getCurBufOffset()
{
	return m_flvWriterOffset;
}

bool flv::FlvWriter::AppendSPSPPS(const char *psps, uint64_t spslen, const char *ppps, uint64_t pppslen, bool flush /*= false*/)
{
	if (m_bHaveSetSPSPPS)
	{
		return true;
	}
	m_bHaveSetSPSPPS = true;
	//previous tag length
	appendint(getLastTagSize());
	//tag type :script data
	appendbyte(flv::tag_type::FLV_TAG_TYPE_VIDEO);
	uint64_t dataSizePos = getCurBufOffset();
	//tag data size
	appendint24(0);  //re-calculate later
	//tag timestamp
	appendint24(0);
	//tag timestamp-ex
	appendbyte(0);
	//tag stream id
	appendint24(0);
	uint64_t dataBufStart = getCurBufOffset();

	appendbyte(flv::FLV_FRAME_KEY);
	//AVC NALU header
	appendbyte(0);
	//composition time
	appendint24(0);

	//version
	appendbyte(1);
	//profile
	appendbyte(psps[4 + 1]);
	//profile
	appendbyte(psps[4 + 2]);
	//profile
	appendbyte(psps[4 + 3]);
	// 6 bits reserved (111111) + 2 bits nal size length - 1 (11)
	appendbyte(0xff);
	// 3 bits reserved (111) + 5 bits number of sps (00001)
	appendbyte(0xe1);

	appendshort(spslen - 4);
	appendString(psps + 4, spslen - 4);

	//pps num
	appendbyte(1);
	//pps len
	appendshort(pppslen - 4);
	//pps data
	appendString(ppps + 4, pppslen - 4);

	uint64_t dataBufEnd = getCurBufOffset();
	uint64_t length = dataBufEnd - dataBufStart;
	rewriteint24(length, dataSizePos);
	setLastTagSize(length + TAG_HEADER_SIZE);
	if (flush)
	{
		flushData();
	}
	return true;
}

void flv::FlvWriter::setScriptParam(uint16_t uiWidth, uint16_t uiHeight, double fps)
{
	m_Width = uiWidth;
	m_Height = uiHeight;
	m_FrameRate = fps;

	m_dts = 0;
	m_cts = 0;
	m_step = 1000 / fps;
}

void flv::FlvWriter::setSoundSize(uint16_t SoundSize)
{
	m_SoundSize = SoundSize;
}

void flv::FlvWriter::setSoundType(uint16_t SoundType)
{
	m_SoundType = SoundType;
}

void flv::FlvWriter::setSoundFormat(uint16_t SoundFormat)
{
	m_SoundFormat = SoundFormat; 
}

void flv::FlvWriter::setSoundRate(uint16_t SoundRate)
{
	m_SoundRate = SoundRate;
}

void flv::FlvWriter::CloseWriter()
{
	fclose(m_file);
}

bool flv::FlvWriter::flushData()
{
	fwrite(m_flvData, m_flvWriterOffset, 1, m_file);
	m_flvWriterOffset = 0;
	return true;
}
