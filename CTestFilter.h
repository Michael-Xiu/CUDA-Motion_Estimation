/*
	Copyright (C) 2015, Liang Fan
	All rights reverved.
*/

/**	\file		CTestFilter.h
	\brief		Header file of CTestFilter class.
*/

#ifndef __CTESTFILTER_H__
#define __CTESTFILTER_H__

#include <streams.h>
#include <initguid.h>
#include <dvdmedia.h>

// {85FE8489-AB5A-4D18-906F-44686964DBCF}
DEFINE_GUID(CLSID_TestFilter,
	0x85fe8489, 0xab5a, 0x4d18, 0x90, 0x6f, 0x44, 0x68, 0x69, 0x64, 0xdb, 0xcf);

// {32595559-0000-0010-8000-00AA00389B71}
DEFINE_GUID(MEDIASUBTYPE_YUY2,
	0x32595559, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

//
#define MAX_WIDTH  1920
#define MAX_HEIGHT 1080

//
// Test Filter class: CTestFilter
//
class CTestFilter : public CTransformFilter
{
public:
	CTestFilter(LPUNKNOWN pUnk, HRESULT *phr);
	~CTestFilter();

	//
	static CUnknown *WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *phr);
	DECLARE_IUNKNOWN;

	//
	HRESULT CheckInputType(const CMediaType *mtIn);
	HRESULT CheckTransform(const CMediaType *mtIn, const CMediaType *mtOut);
	HRESULT GetMediaType(int iPosition, CMediaType *pmt);
	HRESULT DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *ppropInputRequest);

	//
	HRESULT CompleteConnect(PIN_DIRECTION direction, IPin *pReceivePin);

	//
	HRESULT StartStreaming();
	HRESULT StopStreaming();
	HRESULT Receive(IMediaSample *pSample);

protected:
	// critical section
	CCritSec        m_csFilter;

	int             m_iWidth;
	int             m_iHeight;
	LONGLONG        m_lPictureID;
	REFERENCE_TIME  m_rtAvgTimePerFrame;
	REFERENCE_TIME  m_rtStart, m_rtEnd;

	//
	BYTE           *m_pbIn;
	BYTE           *m_pbOut;
	int             m_iInputDataSize;
	int             m_iOutputDataSize;
};

#endif // __CTESTFILTER_H__
