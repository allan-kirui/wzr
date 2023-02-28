#include "stdafx.h"
#include "CPic.h"

bool CPic::LoadDIB(CString sciezka_do_pliku)
{
	if (wskaznik_na_bufor != nullptr)
	{
		HeapFree(GetProcessHeap(), 0, wskaznik_na_bufor);
	}
	CFile f;
	f.Open(sciezka_do_pliku, CFile::modeReadWrite);
	rozmiar_bufora = f.GetLength();
	wskaznik_na_bufor = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, rozmiar_bufora);
	f.Read(wskaznik_na_bufor, rozmiar_bufora);
	int odleglosc = *(int*)(wskaznik_na_bufor + 10);
	bitmapinfo = (tagBITMAPINFO*)(wskaznik_na_bufor + 14);
	bitmapa = (char*)(wskaznik_na_bufor + odleglosc);
	gotowe = true;

	return 0;
}

bool CPic::PaintDIB(HDC kontekst, CRect r, CRect prost_zrodlowy)
{
	SetStretchBltMode(kontekst, COLORONCOLOR);

	int szer = bitmapinfo->bmiHeader.biWidth;
	int wys = bitmapinfo->bmiHeader.biHeight;
	float skala = float(szer) / float(wys);

	if (szer > wys)
	{
		szer = r.right;
		wys = szer / skala;
	}
	else
	{
		wys = r.bottom;
		szer = wys * skala;
	}

	StretchDIBits(kontekst, 0, 0, szer, wys,
		0, 0, bitmapinfo->bmiHeader.biWidth, bitmapinfo->bmiHeader.biHeight,
		bitmapa,
		bitmapinfo,
		DIB_RGB_COLORS,
		SRCCOPY);

	gotowe = false;
	return 0;
}

bool CPic::SaveDIB(CString sciezka_do_pliku)
{
	CFile f;
	f.Open(sciezka_do_pliku, CFile::modeCreate);
	f.Close();
	f.Open(sciezka_do_pliku, CFile::modeReadWrite);
	f.Write(wskaznik_na_bufor, rozmiar_bufora);
	f.Close();
	HeapFree(GetProcessHeap(), 0, wskaznik_na_bufor);
	return 0;
}

bool CPic::CreateGreyscaleDIB(CPic x)
{
	int bajty_linii = (8 * x.bitmapinfo->bmiHeader.biWidth + 31) / 32 * 4;
	int bajty_bitmapy = bajty_linii * x.bitmapinfo->bmiHeader.biHeight;

	rozmiar_bufora2 = 54 + sizeof(tagRGBQUAD) * 256 + bajty_bitmapy;
	wskaznik_na_bufor2 = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, rozmiar_bufora2);

	tagBITMAPFILEHEADER* bitmap_file_header = (tagBITMAPFILEHEADER*)wskaznik_na_bufor2;
	bitmap_file_header->bfType = 0x4d42; // "BM"
	bitmap_file_header->bfSize = rozmiar_bufora2;
	bitmap_file_header->bfOffBits = rozmiar_bufora2 - bajty_bitmapy;

	bitmapinfo = (tagBITMAPINFO*)(wskaznik_na_bufor2 + 14);
	bitmapinfo->bmiHeader = x.bitmapinfo->bmiHeader;
	bitmapinfo->bmiHeader.biBitCount = 8;
	bitmapinfo->bmiHeader.biClrUsed = 256;

	tagRGBQUAD* kolory = (tagRGBQUAD*)(wskaznik_na_bufor2 + 54);
	RGBQUAD kolor_piksela = RGBQUAD();
	for (int i = 0; i < 256; i++)
	{
		kolor_piksela.rgbBlue = i;
		kolor_piksela.rgbGreen = i;
		kolor_piksela.rgbRed = i;
		kolor_piksela.rgbReserved = 0;
		*(kolory + i) = kolor_piksela;
	}

	bitmapa = (wskaznik_na_bufor2 + bitmap_file_header->bfOffBits);
	BYTE val;
	RGBTRIPLE piksel;
	for (int i = 0; i < x.bitmapinfo->bmiHeader.biHeight; i++)
	{
		if (x.bitmapinfo->bmiHeader.biBitCount == 1)
		{
			for (int j = 0; j < bajty_linii; j++)
			{
				if (GetPixel1(j, i, x.bitmapa)) val = (BYTE)255;
				else val = (BYTE)0;
				SetPixel8(j, i, val);
			}
		}
		else if (x.bitmapinfo->bmiHeader.biBitCount == 8) for (int j = 0; j < bajty_linii; j++) SetPixel8(j, i, GetPixel8(j, i, x.bitmapa));
		else
		{
			for (int j = 0; j < bajty_linii; j++)
			{
				piksel = GetPixel24(j, i, x.bitmapa);
				val = (BYTE)(0.299f * (float)piksel.rgbtRed +
					0.587f * (float)piksel.rgbtGreen +
					0.114f * (float)piksel.rgbtBlue);
				SetPixel8(j, i, val);
			}
		}
	}

	gotowe = true;
	return true;
}

bool CPic::GetPixel1(int x, int y, char* bitmapa)
{
	BYTE piksel = (BYTE)(*(bitmapa + Liczba_bajtow(1) * y + (int)(x / 8)));
	return (1 & (piksel >> 7 - x % 8));
}

BYTE CPic::GetPixel8(int x, int y, char* bitmapa)
{
	return (BYTE) * (bitmapa + Liczba_bajtow(8) * y + x);
}

RGBTRIPLE CPic::GetPixel24(int x, int y, char* bitmapa)
{
	RGBTRIPLE piksel;
	int szer = Liczba_bajtow(24);
	piksel.rgbtBlue = (BYTE) * (bitmapa + szer * y + 3 * x);
	piksel.rgbtGreen = (BYTE) * (bitmapa + szer * y + 3 * x + 1);
	piksel.rgbtRed = (BYTE) * (bitmapa + szer * y + 3 * x + 2);
	return piksel;
}

bool CPic::SetPixel8(int x, int y, BYTE val)
{
	*(bitmapa + Liczba_bajtow(8) * y + x) = val;
	return true;
}

int CPic::Liczba_bajtow(int bity_na_piksel)
{
	return (bity_na_piksel * bitmapinfo->bmiHeader.biWidth + 31) / 32 * 4;
}

void CPic::ZmianaJasnosci(int zmiana)
{
	BYTE jasnosc;
	for (int i = 0; i < bitmapinfo->bmiHeader.biHeight; i++)
	{
		for (int j = 0; j < Liczba_bajtow(8); j++)
		{
			jasnosc = GetPixel8(j, i, bitmapa);

			if (int(jasnosc) + zmiana < 0) jasnosc = 0;
			else if (int(jasnosc) + zmiana > 255) jasnosc = 255;
			else jasnosc += zmiana;
			SetPixel8(j, i, jasnosc);
		}
	}

	gotowe = true;
}

void CPic::ZmianaKontrastu(float kontrast)
{
	BYTE jasnosc;
	float zmiana = kontrast - 1.0;
	for (int i = 0; i < bitmapinfo->bmiHeader.biHeight; i++)
	{
		for (int j = 0; j < Liczba_bajtow(8); j++)
		{
			jasnosc = GetPixel8(j, i, bitmapa);
			if (float(jasnosc) * kontrast - 127.0 * zmiana > 255) jasnosc = 255;
			else if (float(jasnosc) * kontrast - 127.0 * zmiana < 0) jasnosc = 0;
			else jasnosc = float(jasnosc) * kontrast - 127.0 * zmiana;
			SetPixel8(j, i, (BYTE)jasnosc);
		}
	}

	gotowe = true;
}

void CPic::Potegowanie(float potega)
{
	BYTE jasnosc;

	for (int i = 0; i < bitmapinfo->bmiHeader.biHeight; i++)
	{
		for (int j = 0; j < Liczba_bajtow(8); j++)
		{
			jasnosc = GetPixel8(j, i, bitmapa);
			SetPixel8(j, i, (BYTE)(pow(float(jasnosc / 255.0), potega) * 255));
		}
	}

	gotowe = true;
}

void CPic::Negatyw()
{
	BYTE jasnosc;
	for (int i = 0; i < bitmapinfo->bmiHeader.biHeight; i++)
	{
		for (int j = 0; j < Liczba_bajtow(8); j++)
		{
			jasnosc = 255 - GetPixel8(j, i, bitmapa);
			SetPixel8(j, i, jasnosc);
		}
	}

	gotowe = true;
}

int* CPic::Histogram()
{
	int* tab = new int[256]{ 0 };
	BYTE jasnosc;
	for (int i = 0; i < bitmapinfo->bmiHeader.biHeight; i++)
	{
		for (int j = 0; j < Liczba_bajtow(8); j++)
		{
			jasnosc = GetPixel8(j, i, bitmapa);
			tab[jasnosc]++;
		}
	}
	return tab;
}
