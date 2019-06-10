// VideoTools.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <vector>
#include <iostream>
#include "Flv.h"

typedef enum frame_type
{
	FRAME_IDR,
	FRAME_SPS,
	FRAME_PPS,
	FRAME_SEI,
	FRAME_NORMAL,
} frame_type;
typedef struct frame_tag
{
	unsigned long long len;
	frame_type frameT;
	char *pBuf;
	
} frame_264;

#define FRAME_BUFFER_LENGTH = 1024 * 1024;

const char *p264file = "F:\\testVideo\\Wildlife.264";
const char *pflvfile = "F:\\testVideo\\Wildlife.flv";

int _tmain(int argc, _TCHAR* argv[])
{
	std::vector<frame_264> frame264;
	FILE * fp = fopen(p264file, "rb");
	if (!fp)
	{
		std::cout << "open 264 failed!" << std::endl;
		system("pause");
		return 0;
	}

	fseek(fp, 0, SEEK_END);
	unsigned long length = ftell(fp);
	rewind(fp);
	char *pbuf = new char[length];
	unsigned long relen = fread(pbuf, 1, length, fp);
	if (relen != length)
	{
		std::cout << "read 264 file failed!" << std::endl;
		system("pause");
		return 0;
	}
	fclose(fp);
	unsigned long offset = 0, framestart = 0, frameend = 0;
	frame_type frameT = FRAME_IDR;
	/* current 264 file has two type identifier: 00 00 00 01  and  00 00 01 */
	while (offset < length)
	{
		if (pbuf[offset] == 0 && pbuf[offset + 1] == 0 && pbuf[offset + 2] == 0 && pbuf[offset + 3] == 1)
		{
			//idr
			if ((pbuf[offset + 4] & 0x0f) == 5)
			{
				if (offset != 0 && frameT != FRAME_SEI)
				{
					frameend = offset - 1;
					frame_264 frame;
					frame.len = frameend - framestart;
					frame.pBuf = new char[frame.len];
					frame.frameT = frameT;
					memcpy(frame.pBuf, &pbuf[framestart], frame.len);
					frame264.push_back(frame);
				}
				framestart = offset;
				frameT = FRAME_IDR;

				offset += 4;
			}
			//sps
			else if ((pbuf[offset + 4] & 0x0f) == 7)
			{
				if (offset != 0 && frameT != FRAME_SEI)
				{
					frameend = offset - 1;
					frame_264 frame;
					frame.len = frameend - framestart;
					frame.pBuf = new char[frame.len];
					frame.frameT = frameT;
					memcpy(frame.pBuf, &pbuf[framestart], frame.len);
					frame264.push_back(frame);
				}
				framestart = offset;
				frameT = FRAME_SPS;

				offset += 4;
			}
			//pps
			else if ((pbuf[offset + 4] & 0x0f) == 8)
			{
				if (offset != 0 && frameT != FRAME_SEI)
				{
					frameend = offset - 1;
					frame_264 frame;
					frame.len = frameend - framestart;
					frame.pBuf = new char[frame.len];
					frame.frameT = frameT;
					memcpy(frame.pBuf, &pbuf[framestart], frame.len);
					frame264.push_back(frame);
				}
				framestart = offset;
				frameT = FRAME_PPS;

				offset += 4;
			}
			else if ((pbuf[offset + 4] & 0x0f) == 6)
			{
				if (offset != 0 && frameT != FRAME_SEI)
				{
					frameend = offset - 1;
					frame_264 frame;
					frame.len = frameend - framestart;
					frame.pBuf = new char[frame.len];
					frame.frameT = frameT;
					memcpy(frame.pBuf, &pbuf[framestart], frame.len);
					frame264.push_back(frame);
				}
				framestart = offset;
				frameT = FRAME_SEI;
			}
			else
			{
				if (offset != 0 && frameT != FRAME_SEI)
				{
					frameend = offset - 1;
					frame_264 frame;
					frame.len = frameend - framestart;
					frame.pBuf = new char[frame.len];
					frame.frameT = frameT;
					memcpy(frame.pBuf, &pbuf[framestart], frame.len);
					frame264.push_back(frame);
				}
				framestart = offset;
				frameT = FRAME_NORMAL;

				offset += 4;
			}
		}
		/*3 byte identifier */
		else if (pbuf[offset] == 0 && pbuf[offset + 1] == 0 && pbuf[offset + 2] == 1)
		{
			//idr
			if ((pbuf[offset + 3] & 0x0f) == 5)
			{
				if (offset != 0 && frameT != FRAME_SEI)
				{
					frameend = offset - 1;
					frame_264 frame;
					frame.len = frameend - framestart;
					frame.pBuf = new char[frame.len];
					frame.frameT = frameT;
					memcpy(frame.pBuf, &pbuf[framestart], frame.len);
					frame264.push_back(frame);
				}
				framestart = offset;
				frameT = FRAME_IDR;

				offset += 3;
			}
			//sps
			else if ((pbuf[offset + 3] & 0x0f) == 7)
			{
				if (offset != 0 && frameT != FRAME_SEI)
				{
					frameend = offset - 1;
					frame_264 frame;
					frame.len = frameend - framestart;
					frame.pBuf = new char[frame.len];
					frame.frameT = frameT;
					memcpy(frame.pBuf, &pbuf[framestart], frame.len);
					frame264.push_back(frame);
				}
				framestart = offset;
				frameT = FRAME_SPS;

				offset += 3;
			}
			//pps
			else if ((pbuf[offset + 3] & 0x0f) == 8)
			{
				if (offset != 0 && frameT != FRAME_SEI)
				{
					frameend = offset - 1;
					frame_264 frame;
					frame.len = frameend - framestart;
					frame.pBuf = new char[frame.len];
					frame.frameT = frameT;
					memcpy(frame.pBuf, &pbuf[framestart], frame.len);
					frame264.push_back(frame);
				}
				framestart = offset;
				frameT = FRAME_PPS;

				offset += 3;
			}
			else if ((pbuf[offset + 3] & 0x0f) == 6)
			{
				if (offset != 0 && frameT != FRAME_SEI)
				{
					frameend = offset - 1;
					frame_264 frame;
					frame.len = frameend - framestart;
					frame.pBuf = new char[frame.len];
					frame.frameT = frameT;
					memcpy(frame.pBuf, &pbuf[framestart], frame.len);
					frame264.push_back(frame);
				}
				framestart = offset;
				frameT = FRAME_SEI;
			}
			else
			{
				if (offset != 0 && frameT != FRAME_SEI)
				{
					frameend = offset - 1;
					frame_264 frame;
					frame.len = frameend - framestart;
					frame.pBuf = new char[frame.len];
					frame.frameT = frameT;
					memcpy(frame.pBuf, &pbuf[framestart], frame.len);
					frame264.push_back(frame);
				}
				framestart = offset;
				frameT = FRAME_NORMAL;

				offset += 3;
			}
		}

		offset++;
	}

	flv::FlvWriter flvWriter;
	flvWriter.CreateWriter(std::string(pflvfile));
	flvWriter.setScriptParam(1280, 720, 25.0);
	flvWriter.setHeadFlag(flv::FLAG_VIDEO);
	flvWriter.setSoundFormat(AUDIO_CODEC_PCMU);
	flvWriter.setSoundRate(AUDIO_SAMPLE_RATE_8K);
	flvWriter.setSoundSize(AUDIO_SAMPLE_SIZE_16);
	flvWriter.setSoundType(AUDIO_MONO);
	flvWriter.WriteFlvHead(true);
	flvWriter.WriteFlvScript(true);

	frame_264 *pSPS = NULL;
	frame_264 *pPPS = NULL;
	auto it = frame264.begin();
	while (it != frame264.end())
	{
		if (it->len == 0)
		{
			it++;
			continue;
		}
		if (it->frameT == FRAME_IDR && pSPS != NULL && pPPS != NULL)
		{
			flvWriter.AppendSPSPPS(pSPS->pBuf, pSPS->len, pPPS->pBuf, pSPS->len, true);
			flvWriter.AppendVideoFrame(it->pBuf, it->len, true);
		}
		else if (it->frameT == FRAME_SPS)
		{
			pSPS = &(*it);
		}
		else if (it->frameT == FRAME_PPS)
		{
			pPPS = &(*it);
		}
		else
		{
			flvWriter.AppendVideoFrame(it->pBuf, it->len, true);
		}
		it++;
	}
	flvWriter.CloseWriter();
	system("pause");
	return 0;
}

