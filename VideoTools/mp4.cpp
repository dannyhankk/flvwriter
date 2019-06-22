#include "stdafx.h"
#include "mp4.h"


bool mp4::Mp4Writer::flushData()
{
	fwrite(m_mp4Data, m_mp4WriterOffset, 1, m_file);
	m_flushLen += m_mp4WriterOffset;
	m_mp4WriterOffset = 0;
	return true;
}

uint64_t mp4::Mp4Writer::getCurBufOffset()
{
	return m_mp4WriterOffset;
}

bool mp4::Mp4Writer::reAllocMem(uint64_t needSize)
{
	printf("re-alloc memory\n");
	m_mp4DataLen = (m_mp4DataLen > needSize ? m_mp4DataLen : needSize) * DEFAULT_BUFFER_INCR_FACTOR;
	/* increase memory just after origin memory*/
	uint8_t *newMem = (uint8_t *)realloc(m_mp4Data, m_mp4DataLen);
	if (newMem == nullptr)
	{

		/* try a new memory block */
		newMem = (uint8_t *)malloc(m_mp4DataLen);
		if (newMem == nullptr)
		{
			printf("re-alloc memory failed\n");
			return false;
		}
		memcpy(newMem, m_mp4Data, m_mp4WriterOffset);
		free(m_mp4Data);
		m_mp4Data = newMem;
		return true;
	}
	return true;
}

bool mp4::Mp4Writer::checkAndreAllcoMem(uint64_t needSize)
{
	if (m_mp4WriterOffset + needSize < m_mp4DataLen)
	{
		return true;
	}
	return reAllocMem(needSize);
}

bool mp4::Mp4Writer::CreateWriter(std::string & mp4Name)
{
	m_file = fopen(mp4Name.c_str(), "wb");
	if (m_file == nullptr)
	{
		printf("open file %s failed\n", mp4Name.c_str());
		return false;
	}

	if (m_mp4Data == nullptr)
	{
		m_mp4DataLen = sizeof(uint8_t)* DEFAULT_BUFFER_LEN;
		m_mp4Data = (uint8_t *)malloc(m_mp4DataLen);

		if (m_mp4Data == nullptr)
		{
			m_mp4Data = nullptr;
			m_mp4DataLen = 0;
			printf("malloc buffer failed!\n");
			return false;
		}
		m_mp4WriterOffset = 0;
	}
	return true;
}

void mp4::Mp4Writer::CloseWriter()
{
	fclose(m_file);
}

bool mp4::Mp4Writer::WriteFileTypeBox(bool bFlush)
{
	m_ftypBox.stHead.boxSize = util::toBigEndian((uint32_t)sizeof(m_ftypBox));
	memcpy(m_ftypBox.stHead.boxType, mp4::pftyp, sizeof(m_ftypBox.stHead.boxType));
	memcpy(m_ftypBox.majorBrand, mp4::pisom, sizeof(m_ftypBox.majorBrand));
	m_ftypBox.minorVersion = util::toBigEndian((uint32_t)DEFAULT_VERSION);

	memcpy((char *)&m_ftypBox.compatibleBrands[0], pisom, DEFAULT_BRAND_LEN);
	memcpy((char *)&m_ftypBox.compatibleBrands[1], piso2, DEFAULT_BRAND_LEN);
	memcpy((char *)&m_ftypBox.compatibleBrands[2], pavc1, DEFAULT_BRAND_LEN);
	memcpy((char *)&m_ftypBox.compatibleBrands[3], pmp41, DEFAULT_BRAND_LEN);

	AppendString((char *)&m_ftypBox, sizeof(m_ftypBox));
	if (bFlush)
	{
		flushData();
	}
	return true;
}

bool mp4::Mp4Writer::AppendString(const char *pbuf, uint32_t len)
{
	if (!checkAndreAllcoMem(len))
	{
		printf("memory error, append uint64_t failed\n");
		return;
	}

	memcpy(m_mp4Data + m_mp4WriterOffset, pbuf, len);
	m_mp4WriterOffset += len;
	return true;
}

bool mp4::Mp4Writer::WriteKeyFrame(const char *pbuf, uint32_t len, bool bFlush /*= false*/)
{
	// record every sample size
	m_vSampleSize.push_back(len);

	// key frame increase and key frame index
	m_stssBox.boxEntryCount += 1;
	m_vKeySampleCount.push_back(m_vSampleSize.size());


	// need a new chunk
	if (m_vSampleSize.size() == 0 || m_samplePerChunk > DEFAULT_CHUNK_SAMPLE_NUM )
	{
		// add new chunk offset
		m_vChunkOffset.push_back(m_mp4WriterOffset + m_flushLen);	
		// add new sample chunk offset
		stsc_Data tmp;
		tmp.firstChunk = m_vStsc.size() + 1;
		tmp.samplePerChunk = m_samplePerChunk;
		tmp.sampleDesIndex = 1;
		m_vStsc.push_back(tmp);
	}
	AppendString(pbuf, len);
	m_samplePerChunk += 1;
	
	if (bFlush)
	{
		flushData();
	}

	return true;
}

bool mp4::Mp4Writer::WriteNormalFrame(const char *pbuf, uint32_t len, bool bFlush /*= false*/)
{
	// record every sample size
	m_vSampleSize.push_back(len);

	// need a new chunk
	if (m_vSampleSize.size() == 0 || m_samplePerChunk > DEFAULT_CHUNK_SAMPLE_NUM)
	{
		// add new chunk offset
		m_vChunkOffset.push_back(m_mp4WriterOffset + m_flushLen);
		// add new sample chunk offset
		stsc_Data tmp;
		tmp.firstChunk = m_vStsc.size() + 1;
		tmp.samplePerChunk = m_samplePerChunk;
		tmp.sampleDesIndex = 1;
		m_vStsc.push_back(tmp);
	}
	AppendString(pbuf, len);
	m_samplePerChunk += 1;

	if (bFlush)
	{
		flushData();
	}

	return true;
}

bool mp4::Mp4Writer::WriteMoovBox(bool bFulsh /*= false*/)
{

}
