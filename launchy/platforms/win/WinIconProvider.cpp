/*
Launchy: Application Launcher
Copyright (C) 2007-2009  Josh Karlin, Simon Capewell

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/


#include "precompiled.h"
#include "platform_win_util.h"
#include "WinIconProvider.h"

#if QT_VERSION >= 0x050000
#   include <QtWinExtras/QtWinExtras>
#endif

// Temporary work around to avoid having to install the latest Windows SDK
#ifndef __IShellItemImageFactory_INTERFACE_DEFINED__
#define __IShellItemImageFactory_INTERFACE_DEFINED__

#define SHIL_JUMBO 0x4 
/* IShellItemImageFactory::GetImage() flags */
enum _SIIGB {
    SIIGBF_RESIZETOFIT      = 0x00000000,
    SIIGBF_BIGGERSIZEOK     = 0x00000001,
    SIIGBF_MEMORYONLY       = 0x00000002,
    SIIGBF_ICONONLY         = 0x00000004,
    SIIGBF_THUMBNAILONLY    = 0x00000008,
    SIIGBF_INCACHEONLY      = 0x00000010
};
typedef int SIIGBF;


const GUID IID_IShellItemImageFactory = {0xbcc18b79,0xba16,0x442f,{0x80,0xc4,0x8a,0x59,0xc3,0x0c,0x46,0x3b}};

class IShellItemImageFactory : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE GetImage(SIZE size, SIIGBF flags, /*__RPC__deref_out_opt*/ HBITMAP *phbm) = 0;
};

#endif

HRESULT (WINAPI* fnSHCreateItemFromParsingName)(PCWSTR, IBindCtx *, REFIID, void **) = NULL;


WinIconProvider::WinIconProvider() :
	preferredSize(32)
{
	// Load Vista/7 specific API pointers
	HMODULE hLib = GetModuleHandleW(L"shell32");
	if (hLib)
	{
		(FARPROC&)fnSHCreateItemFromParsingName = GetProcAddress(hLib, "SHCreateItemFromParsingName");
	}
}


WinIconProvider::~WinIconProvider()
{
}


void WinIconProvider::setPreferredIconSize(int size)
{
	preferredSize = size;
}


// This also exists in plugin_interface, need to remove both if I make a 64 build
QString wicon_aliasTo64(QString path) 
{
	QProcessEnvironment env = QProcessEnvironment::systemEnvironment ();
	QString pf32 = env.value("PROGRAMFILES");
	QString pf64 = env.value("PROGRAMW6432");

	// On 64 bit windows, 64 bit shortcuts don't resolve correctly from 32 bit executables, fix it here
	QFileInfo pInfo(path);

	if (env.contains("PROGRAMW6432") && pInfo.isSymLink() && pf32 != pf64) {
		if (QDir::toNativeSeparators(pInfo.symLinkTarget()).contains(pf32)) {
			QString path64 = QDir::toNativeSeparators(pInfo.symLinkTarget());
			path64.replace(pf32, pf64);
			if (QFileInfo(path64).exists()) {
				path = path64;
			}
		}
		else if (pInfo.symLinkTarget().contains("system32")) {
			QString path32 = QDir::toNativeSeparators(pInfo.symLinkTarget());
			if (!QFileInfo(path32).exists()) {
				path = path32.replace("system32", "sysnative");
			}
		}
	}
	return path;
}

QIcon WinIconProvider::icon(const QFileInfo& info) const
{
	QIcon retIcon;
	QString fileExtension = info.suffix().toLower();

	if (fileExtension == "png" ||
		fileExtension == "bmp" ||
		fileExtension == "jpg" ||
		fileExtension == "jpeg")
	{
		retIcon = QIcon(info.filePath());
	}
	else if (fileExtension == "cpl")
	{
		HICON hIcon;
		QString filePath = QDir::toNativeSeparators(info.filePath());
        ExtractIconEx(reinterpret_cast<const wchar_t*>(filePath.utf16()), 0, &hIcon, NULL, 1);

#if QT_VERSION >= 0x050000
        retIcon = QIcon(QtWin::fromHICON(hIcon));
#else
        retIcon = QIcon(QPixmap::fromWinHICON(hIcon));
#endif
		DestroyIcon(hIcon);
	}
	else
	{
		// This 64 bit mapping needs to go away if we produce a 64 bit build of launchy
		QString filePath = wicon_aliasTo64(QDir::toNativeSeparators(info.filePath()));

		// Get the icon index using SHGetFileInfo
		SHFILEINFO sfi = {0};

		QRegExp re("\\\\\\\\([a-z0-9\\-]+\\\\?)?$", Qt::CaseInsensitive);
		if (re.exactMatch(filePath))
		{
			// To avoid network hangs, explicitly fetch the My Computer icon for UNCs
			LPITEMIDLIST pidl;
			if (SHGetSpecialFolderLocation(NULL, CSIDL_DRIVES, &pidl) == S_OK)
			{
				SHGetFileInfo((LPCTSTR)pidl, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_SYSICONINDEX);
				// Set the file path to the My Computer GUID for any later fetches
				filePath = "::{20D04FE0-3AEA-1069-A2D8-08002B30309D}";
			}
		}
		if (sfi.iIcon == 0)
		{
            SHGetFileInfo(reinterpret_cast<const wchar_t*>(filePath.utf16()), 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX);
		}

		// An icon index of 3 is the generic file icon
		if (sfi.iIcon > 0 && sfi.iIcon != 3)
		{
			// Retrieve the system image list.
			// To get the 48x48 icons, use SHIL_EXTRALARGE
			// To get the 256x256 icons (Vista only), use SHIL_JUMBO
			int imageListIndex;
			if (preferredSize <= 16)
				imageListIndex = SHIL_SMALL;
			else if (preferredSize <= 32)
				imageListIndex = SHIL_LARGE;
			else if (preferredSize <= 48)
				imageListIndex = SHIL_EXTRALARGE;
			else
				imageListIndex = SHIL_JUMBO;

			// If the OS supports SHCreateItemFromParsingName, get a 256x256 icon
			if (!addIconFromShellFactory(filePath, retIcon))
			{
				// otherwise get the largest appropriate size
				if (!addIconFromImageList(imageListIndex, sfi.iIcon, retIcon) && imageListIndex == SHIL_JUMBO)
					addIconFromImageList(SHIL_EXTRALARGE, sfi.iIcon, retIcon);
			}

			// Ensure there's also a 32x32 icon - extralarge and above often only contain
			// a large frame with the 32x32 icon in the middle or looks blurry
			if (imageListIndex == SHIL_EXTRALARGE || imageListIndex == SHIL_JUMBO)
				addIconFromImageList(SHIL_LARGE, sfi.iIcon, retIcon);
		}
		else if (info.isSymLink() || fileExtension == "lnk") // isSymLink is case sensitive when it perhaps shouldn't be
		{
			QFileInfo targetInfo(info.symLinkTarget());
			retIcon = icon(targetInfo);
		}
		else
		{
			retIcon = QFileIconProvider::icon(info);
		}		
	}

	return retIcon;
}


bool WinIconProvider::addIconFromImageList(int imageListIndex, int iconIndex, QIcon& icon) const
{
	HICON hIcon = 0;
	IImageList* imageList;
	HRESULT hResult = SHGetImageList(imageListIndex, IID_IImageList, (void**)&imageList);
	if (hResult == S_OK)
	{
		hResult = ((IImageList*)imageList)->GetIcon(iconIndex, ILD_TRANSPARENT, &hIcon);
		imageList->Release();
	}
	if (hResult == S_OK && hIcon)
	{
#if QT_VERSION >= 0x050000
        icon.addPixmap(QtWin::fromHICON(hIcon));
#else
        icon.addPixmap(QPixmap::fromWinHICON(hIcon));
#endif

		DestroyIcon(hIcon);
	}

	return SUCCEEDED(hResult);
}


/*
 * anonymous namespace contains helper functions from gui/image/qpixmap_win.cpp used by fixed_qt_pixmapFromWinHBitmap
 */
namespace{
    /*
     * from gui/image/qpixmap_win.cpp
     */
    static inline void initBitMapInfoHeader(int width, int height, bool topToBottom, BITMAPINFOHEADER *bih)
    {
        memset(bih, 0, sizeof(BITMAPINFOHEADER));
        bih->biSize        = sizeof(BITMAPINFOHEADER);
        bih->biWidth       = width;
        bih->biHeight      = topToBottom ? -height : height;
        bih->biPlanes      = 1;
        bih->biBitCount    = 32;
        bih->biCompression = BI_RGB;
        bih->biSizeImage   = width * height * 4;
    }

    /*
     * from gui/image/qpixmap_win.cpp
     */
    static inline void initBitMapInfo(int width, int height, bool topToBottom, BITMAPINFO *bmi)
    {
        initBitMapInfoHeader(width, height, topToBottom, &bmi->bmiHeader);
        memset(bmi->bmiColors, 0, sizeof(RGBQUAD));
    }

    /*
     * from gui/image/qpixmap_win.cpp
     */
    static inline uchar *getDiBits(HDC hdc, HBITMAP bitmap, int width, int height, bool topToBottom = true)
    {
        BITMAPINFO bmi;
        initBitMapInfo(width, height, topToBottom, &bmi);
        uchar *result = new uchar[bmi.bmiHeader.biSizeImage];
        if (!GetDIBits(hdc, bitmap, 0, height, result, &bmi, DIB_RGB_COLORS)) {
            delete [] result;
            qErrnoWarning("%s: GetDIBits() failed to get bitmap bits.", __FUNCTION__);
            return 0;
        }
        return result;
    }

    /*
     * from gui/image/qpixmap_win.cpp
     */
    static inline void copyImageDataCreateAlpha(const uchar *data, QImage *target)
    {
        const uint mask = target->format() == QImage::Format_RGB32 ? 0xff000000 : 0;
        const int height = target->height();
        const int width = target->width();
        const int bytesPerLine = width * int(sizeof(QRgb));
        for (int y = 0; y < height; ++y) {
            QRgb *dest = reinterpret_cast<QRgb *>(target->scanLine(y));
            const QRgb *src = reinterpret_cast<const QRgb *>(data + y * bytesPerLine);
            for (int x = 0; x < width; ++x) {
                const uint pixel = src[x];
                if ((pixel & 0xff000000) == 0 && (pixel & 0x00ffffff) != 0)
                    dest[x] = pixel | 0xff000000;
                else
                    dest[x] = pixel | mask;
            }
        }
    }
}

/*
 * fixed version of qt_pixmapFromWinHBitmap
 */
QPixmap fixed_qt_pixmapFromWinHBitmap(HBITMAP bitmap, int hbitmapformat = 0){
    // Verify size
    BITMAP bitmap_info;
    memset(&bitmap_info, 0, sizeof(BITMAP));

    const int res = GetObject(bitmap, sizeof(BITMAP), &bitmap_info);
    if (!res) {
        qErrnoWarning("QPixmap::fromWinHBITMAP(), failed to get bitmap info");
        return QPixmap();
    }
    const int w = bitmap_info.bmWidth;
    const int h = bitmap_info.bmHeight;

    // Get bitmap bits
    HDC display_dc = GetDC(0);
    QScopedArrayPointer<uchar> data(getDiBits(display_dc, bitmap, w, h, true));
    if (data.isNull()) {
        ReleaseDC(0, display_dc);
        return QPixmap();
    }
    /********
     * BEGIN Changed Code
     ********/
    const QImage::Format imageFormat = hbitmapformat == QtWin::HBitmapNoAlpha ?
                QImage::Format_RGB32 : hbitmapformat == QtWin::HBitmapPremultipliedAlpha ?
                    QImage::Format_ARGB32_Premultiplied : QImage::Format_ARGB32;
    /********
     * END Changed Code
     ********/

    // Create image and copy data into image.
    QImage image(w, h, imageFormat);
    if (image.isNull()) { // failed to alloc?
        ReleaseDC(0, display_dc);
        qWarning("%s, failed create image of %dx%d", __FUNCTION__, w, h);
        return QPixmap();
    }
    copyImageDataCreateAlpha(data.data(), &image);
    ReleaseDC(0, display_dc);
    return QPixmap::fromImage(image);
}


// On Vista or 7 we could use SHIL_JUMBO to get a 256x256 icon,
// but we'll use SHCreateItemFromParsingName as it'll give an identical
// icon to the one shown in explorer and it scales automatically.
bool WinIconProvider::addIconFromShellFactory(QString filePath, QIcon& icon) const
{
	HRESULT hr = S_FALSE;

	if (fnSHCreateItemFromParsingName)
	{
		IShellItem* psi = NULL;
        hr = fnSHCreateItemFromParsingName(reinterpret_cast<const wchar_t*>(filePath.utf16()), 0, IID_IShellItem, (void**)&psi);
		if (hr == S_OK)
		{
			IShellItemImageFactory* psiif = NULL;
			hr = psi->QueryInterface(IID_IShellItemImageFactory, (void**)&psiif);
			if (hr == S_OK)
			{
				HBITMAP iconBitmap = NULL;
				SIZE iconSize = {preferredSize, preferredSize};
				hr = psiif->GetImage(iconSize, SIIGBF_RESIZETOFIT | SIIGBF_ICONONLY, &iconBitmap);
				if (hr == S_OK)
				{
#if QT_VERSION >= 0x050000
                    QPixmap iconPixmap = fixed_qt_pixmapFromWinHBitmap(iconBitmap, QtWin::HBitmapAlpha);
#else
                    QPixmap iconPixmap = QPixmap::fromWinHBITMAP(iconBitmap, QPixmap::PremultipliedAlpha);
#endif
					icon.addPixmap(iconPixmap);
					DeleteObject(iconBitmap);
				}

				psiif->Release();
			}
			psi->Release();
		}
	}

	return hr == S_OK;
}
