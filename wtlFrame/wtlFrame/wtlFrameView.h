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
#include "anim_util.h"
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

    
    LRESULT OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        if ((mux != NULL) && (WEBP_TIMER == (int32_t)wParam))
        {
            ++m_frameIndex;
            if (m_frameIndex >= anim_info.frame_count)
            {
                m_frameIndex = 0;
            }

            LoadFrameData(m_frameIndex);
            InvalidateRect(NULL);
        }
        return 0;
    }
        LRESULT OnDropFiles(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        HDROP hDropInfo = (HDROP)wParam;

        TCHAR szPath[MAX_PATH + 1];

        UINT nCount = ::DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);
        if (nCount)
        {
            for (UINT nIndex = 0; nIndex < nCount; ++nIndex)
            {
                ::DragQueryFile(hDropInfo, nIndex, szPath, _countof(szPath));

                LoadFile(szPath);
                break;
            }
        }

        ::DragFinish(hDropInfo);   
        return 0;
    }


    int ImgIoUtilReadFile(const wchar_t* const file_name, const uint8_t** data, size_t* data_size)
    {
        int ok;
        void* file_data;
        size_t file_size;
        FILE* in = NULL;

        if (data == NULL || data_size == NULL) return 0;
        *data = NULL;
        *data_size = 0;

        _wfopen_s(&in, file_name, _T("rb"));
        if (in == NULL) {
            return 0;
        }
        fseek(in, 0, SEEK_END);
        file_size = ftell(in);
        fseek(in, 0, SEEK_SET);
        file_data = malloc(file_size);
        if (file_data == NULL) return 0;
        ok = (fread(file_data, file_size, 1, in) == 1);
        fclose(in);

        if (!ok)
        {
            free(file_data);
            return 0;
        }
        *data = (uint8_t*)file_data;
        *data_size = file_size;
        return 1;
    }

    WebPMux* mux;
    int LoadWebP(const wchar_t* const in_file,  const uint8_t** data, size_t* data_size, WebPBitstreamFeatures* bitstream) 
    {
        VP8StatusCode status;
        WebPBitstreamFeatures local_features;
        if (!ImgIoUtilReadFile(in_file, data, data_size)) return 0;

        if (bitstream == NULL) {
            bitstream = &local_features;
        }

        status = WebPGetFeatures(*data, *data_size, bitstream);
        if (status != VP8_STATUS_OK) {
            free((void*)*data);
            *data = NULL;
            *data_size = 0;
            return 0;
        }
        return 1;
    }

    void LoadFrameData(int32_t index)
    {
        WebPMuxFrameInfo image;
        WebPDataInit(&image.bitstream);
        WebPMuxGetFrame(mux, index, &image);
        SetTimer(WEBP_TIMER, image.duration, NULL);

        int32_t width = m_szAnim.cx;
        int32_t height = m_szAnim.cy;
        uint8_t* pBGRAData = WebPDecodeBGRA(image.bitstream.bytes, image.bitstream.size, &width, &height);
        if (pBGRAData != NULL)
        {
            if (!m_bimap.IsNull())
            {
                m_bimap.DeleteObject();
            }
            m_bimap.CreateBitmap(width, height, 1, 32, pBGRAData);
        }

        WebPDataClear(&image.bitstream);
        delete[] pBGRAData;
    }

    int32_t m_frameIndex;
    WebPAnimInfo anim_info;
    void LoadFile(wchar_t* filePath)
    {
        WebPDecoderConfig config;
        WebPDecBuffer* const output_buffer = &config.output;
        WebPBitstreamFeatures* const bitstream = &config.input;
        uint8_t* external_buffer = NULL;
        int use_external_memory = 0;
        const uint8_t* data = NULL;
        size_t data_size = 0;
        LoadWebP(filePath, &data, &data_size, bitstream);

        int32_t width = 0;
        int32_t height = 0;
        if (WebPGetInfo(data, data_size, &width, &height))
        {
        }
        uint8_t* pBGRAData = NULL;
        if (!bitstream->has_animation)
        {
            pBGRAData = WebPDecodeBGRA(data, data_size, &width, &height);

            if (pBGRAData != NULL)
            {
                if (!m_bimap.IsNull())
                {
                    m_bimap.DeleteObject();
                }
                m_bimap.CreateBitmap(width, height, 1, 32, pBGRAData);
                delete[] pBGRAData;
            }
        }
        else
        {
            WebPData webp_data;
            if (mux != NULL)
            {
                WebPMuxDelete(mux);
            }
            m_szAnim = CSize(width, height);

            WebPDataInit(&webp_data);
            webp_data.size = data_size;
            webp_data.bytes = data;
            WebPAnimDecoderOptions dec_options;
            WebPAnimDecoderOptionsInit(&dec_options);
            // Tune 'dec_options' as needed.
            WebPAnimDecoder* dec = WebPAnimDecoderNew(&webp_data, &dec_options);
            WebPAnimDecoderGetInfo(dec, &anim_info);
            WebPAnimDecoderDelete(dec);
            int copy_data = 0;
            m_frameIndex = 0;
            // ... (Read data from file).
            mux = WebPMuxCreate(&webp_data, copy_data);
            LoadFrameData(m_frameIndex);
        }

        InvalidateRect(NULL);
    }

    LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        mux = NULL;
        LoadFile(_T("2_webp_ll.webp"));
        return 0;
    }
        LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        CPaintDC dc(m_hWnd);
        CRect rcClient;
        GetClientRect(&rcClient);

//         CMemoryDC dc(paintDC, paintDC.m_ps.rcPaint);
//         dc.FillSolidRect(&rcClient, 0xFFFFFF);
        
        if (!m_bimap.IsNull())
        {
            BITMAP bm;
            m_bimap.GetBitmap(&bm);

            CDC dc1;
            dc1.CreateCompatibleDC(dc);
            HBITMAP pOldBmp = (HBITMAP)dc1.SelectBitmap(m_bimap);

            CRect rcBitmap(0, 0, bm.bmWidth, bm.bmHeight);
            rcBitmap.OffsetRect(rcClient.CenterPoint() - rcBitmap.CenterPoint());

            BLENDFUNCTION blend = { 0 };
            blend.BlendOp = AC_SRC_OVER;
            blend.AlphaFormat = AC_SRC_ALPHA;
            blend.SourceConstantAlpha = 255;
            dc.AlphaBlend(rcBitmap.left, rcBitmap.top, bm.bmWidth, bm.bmHeight, dc1, 0, 0, bm.bmWidth, bm.bmHeight, blend);
            dc1.SelectBitmap(pOldBmp);
        }

        return 0;
	}
};
