/*

Usage:

-filename "my screeny.png"    file name (default: screenshot.png)
-encoder png                  file encoder: bmp/jpeg/gif/tiff/png (default: png)
-quality 100                  file quality for jpeg (between 0 and 100)
-resize 50                    image size, % of the original size (between 1 and 99)

*/

#include <windows.h>
#include <gdiplus.h>

using namespace Gdiplus;

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
Bitmap* ResizeBitmap(Bitmap* source, int percentage);

int APIENTRY wWinMain(HINSTANCE hInstance,
					  HINSTANCE hPrevInstance,
					  LPWSTR    lpCmdLine,
					  int       nCmdShow)
{
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	HWND desktop = GetDesktopWindow();
	HDC desktopdc = GetDC(desktop);
	HDC mydc = CreateCompatibleDC(desktopdc);
	int width = GetSystemMetrics(SM_CXSCREEN);
	int height = GetSystemMetrics(SM_CYSCREEN);
	HBITMAP mybmp = CreateCompatibleBitmap(desktopdc, width, height);
	HBITMAP oldbmp = (HBITMAP)SelectObject(mydc, mybmp);
	BitBlt(mydc,0,0,width,height,desktopdc,0,0, SRCCOPY|CAPTUREBLT);
	SelectObject(mydc, oldbmp);

	wchar_t* filename = L"screenshot.png";
	wchar_t encoder[16] = L"image/png";
	long quality = -1;
	int resize = -1;

	for(int i=1; i<__argc-1; i++)
	{
		if(wcscmp(__wargv[i], L"-filename") == 0)
			filename = __wargv[++i];
		else if(wcscmp(__wargv[i], L"-encoder") == 0)
			wcsncpy_s(encoder+wcslen(L"image/"), 16-wcslen(L"image/"), __wargv[++i], _TRUNCATE);
		else if(wcscmp(__wargv[i], L"-quality") == 0)
			quality = _wtoi(__wargv[++i]);
		else if(wcscmp(__wargv[i], L"-resize") == 0)
			resize = _wtoi(__wargv[++i]);
	}

	Bitmap* b = Bitmap::FromHBITMAP(mybmp, NULL);
	CLSID encoderClsid;
	EncoderParameters encoderParameters;
	Status stat = GenericError;

	if(resize > 0 && resize < 100 && b)
	{
		Bitmap* temp = ResizeBitmap(b, resize);
		delete b;
		b = temp;
	}

	if(b)
	{
		if(GetEncoderClsid(encoder, &encoderClsid) != -1)
		{
			if(quality >= 0 && quality <= 100 && wcscmp(encoder, L"image/jpeg") == 0)
			{
				encoderParameters.Count = 1;
				encoderParameters.Parameter[0].Guid = EncoderQuality;
				encoderParameters.Parameter[0].Type = EncoderParameterValueTypeLong;
				encoderParameters.Parameter[0].NumberOfValues = 1;
				encoderParameters.Parameter[0].Value = &quality;

				stat = b->Save(filename, &encoderClsid, &encoderParameters);
			}
			else
				stat = b->Save(filename, &encoderClsid, NULL);
		}

		delete b;
	}

	// cleanup
	GdiplusShutdown(gdiplusToken);
	ReleaseDC(desktop, desktopdc);
	DeleteObject(mybmp);
	DeleteDC(mydc);
	return (stat == Ok)?0:1;
}

// From http://msdn.microsoft.com/en-us/library/ms533843%28VS.85%29.aspx
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if(size == 0)
		return -1;  // Failure

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if(pImageCodecInfo == NULL)
		return -1;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for(UINT j = 0; j < num; ++j)
	{
		if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}    
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}

Bitmap* ResizeBitmap(Bitmap* source, int percentage)
{
	int w = source->GetWidth()*percentage/100;
	if(w < 1)
		w = 1;

	int h = source->GetHeight()*percentage/100;
	if(h < 1)
		h = 1;

	Bitmap* b = new Bitmap(w, h, source->GetPixelFormat());
	if(b)
	{
		Graphics* g = Graphics::FromImage(b);
		if(g)
		{
			g->DrawImage(source, 0, 0, w, h);
			delete g;
		}
		else
		{
			delete b;
			b = NULL;
		}
	}

	return b;
}
