#pragma once
#include "CPic.h"
// CImgWnd

class CImgWnd : public CWnd
{
	DECLARE_DYNAMIC(CImgWnd)

public:
	CPic x;

	CImgWnd();
	virtual ~CImgWnd();

	BOOL Create(const RECT& rect, CWnd*  pParentWnd, UINT nID);

protected:
	

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
};


