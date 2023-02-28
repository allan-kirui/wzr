#pragma once
class CPic
{
public:
	int* tab;
	bool gotowe = false;
	tagBITMAPINFO* bitmapinfo = nullptr;
	tagBITMAPINFO* bitmapinfo2 = nullptr;
	char* bitmapa = nullptr;
	char* wskaznik_na_bufor = nullptr;
	char* wskaznik_na_bufor2 = nullptr;
	int rozmiar_bufora = 0, rozmiar_bufora2 = 0;
	bool LoadDIB(CString sciezka_do_pliku);
	bool PaintDIB(HDC kontekst, CRect r, CRect prost_zrodlowy);
	bool SaveDIB(CString sciezka_do_pliku);
	bool CreateGreyscaleDIB(CPic x);
	bool GetPixel1(int x, int y, char* pixels);
	BYTE GetPixel8(int x, int y, char* pixels);
	RGBTRIPLE GetPixel24(int x, int y, char* pixels);
	bool SetPixel8(int x, int y, BYTE value);
	int Liczba_bajtow(int bitCount);
	void ZmianaJasnosci(int jasnosc);
	void ZmianaKontrastu(float kontrast);
	void Potegowanie(float potega);
	void Negatyw();
	int* Histogram();
};

