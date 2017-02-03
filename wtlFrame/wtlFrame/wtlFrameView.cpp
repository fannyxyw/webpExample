// wtlFrame.cpp : main source file for wtlFrame.exe
//

#include "stdafx.h"

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>

#include "wtlFrameView.h"

LRESULT CWtlFrameView::OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    if ((m_pWebMux != NULL) && (WEBP_TIMER == (int32_t)wParam))
    {
        ++m_frameIndex;
        if (m_frameIndex >= (int32_t)anim_info.frame_count)
        {
            m_frameIndex = 0;
        }

        if (m_frameIndex >= 0)
        {
            ::MessageBox(0, 0, 0, 0);

        }
        LoadFrameData(m_frameIndex);
        InvalidateRect(NULL);
    }
    return 0;
}

LRESULT CWtlFrameView::OnDropFiles(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
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

int CWtlFrameView::ImgIoUtilReadFile(const wchar_t* const file_name, const uint8_t** data, size_t* data_size)
{
    int ok;
    void* file_data;
    size_t file_size;
    FILE* in = NULL;

    if (data == NULL || data_size == NULL)
        return 0;
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

int CWtlFrameView::LoadWebP(const wchar_t* const in_file, const uint8_t** data, size_t* data_size, WebPBitstreamFeatures* bitstream)
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

void CWtlFrameView::LoadFrameData(int32_t index)
{
    WebPMuxFrameInfo image;
    WebPDataInit(&image.bitstream);
    WebPMuxGetFrame(m_pWebMux, index, &image);
    SetTimer(WEBP_TIMER, image.duration, NULL);

    AtlTrace(_T("output success SDFSDFSDF\n"));

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

void CWtlFrameView::LoadFile(wchar_t* filePath)
{
    KillTimer(WEBP_TIMER);

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
        if (m_pWebMux != NULL)
        {
            WebPMuxDelete(m_pWebMux);
            m_pWebMux = NULL;
        }
        m_szAnim = CSize(width, height);

        WebPDataInit(&webp_data);
        webp_data.size = data_size;
        webp_data.bytes = data;
        WebPAnimDecoderOptions dec_options;
        WebPAnimDecoderOptionsInit(&dec_options);
        WebPAnimDecoder* dec = WebPAnimDecoderNew(&webp_data, &dec_options);
        WebPAnimDecoderGetInfo(dec, &anim_info);
        WebPAnimDecoderDelete(dec);
        int copy_data = 0;
        m_frameIndex = 0;
        m_pWebMux = WebPMuxCreate(&webp_data, copy_data);
        LoadFrameData(m_frameIndex);
    }

    InvalidateRect(NULL);
}

LRESULT CWtlFrameView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    m_pWebMux = NULL;
    LoadFile(_T("2_webp_ll.webp"));
    return 0;
}
LRESULT CWtlFrameView::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
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
