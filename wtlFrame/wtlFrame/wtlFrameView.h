// wtlFrameView.h : interface of the CWtlFrameView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#pragma comment(lib, "libwebpdecoder.lib")
#pragma comment(lib, "libwebpmux.lib")
#pragma comment(lib, "libwebpdemux.lib")

#include "webp/decode.h"
#include "webp/mux.h"
#include "webp/demux.h"
#include <atltypes.h>
#include <atlfile.h>
#include <complex>

#define WEBP_TIMER WM_USER + 1000

class CWtlFrameView : public CWindowImpl<CWtlFrameView>
{
public:
	DECLARE_WND_CLASS(NULL)

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		pMsg;
		return FALSE;
	}

    CBitmap m_bimap;
    CSize m_szAnim;

	BEGIN_MSG_MAP(CWtlFrameView)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_TIMER, OnTimer)
        MESSAGE_HANDLER(WM_DROPFILES, OnDropFiles)
    END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    //	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

    
    LRESULT OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnDropFiles(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);

    int ImgIoUtilReadFile(const wchar_t* const file_name, const uint8_t** data, size_t* data_size);
   

    WebPMux* m_pWebMux;
    int LoadWebP(const wchar_t* const in_file, const uint8_t** data, size_t* data_size, WebPBitstreamFeatures* bitstream);
    void LoadFrameData(int32_t index);
    

    int32_t m_frameIndex;
    WebPAnimInfo anim_info;
    void LoadFile(wchar_t* filePath);
    

    LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
};
