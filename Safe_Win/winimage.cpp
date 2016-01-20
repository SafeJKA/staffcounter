/**********************************************************************
** Copyright (C) 2009-2016 Tesline-Service S.R.L.  All rights reserved.
**
** StaffCounter Agent for Windows 
** 
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://StaffCounter.net/ for GPL licensing information.
**
** Contact info@rohos.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "stdafx.h"
#include <windows.h>
#include <fstream>
#include "winimage.h"

using namespace std;

// Error processing macro (NO-OP by default):
#define ERRORMSG(PARAM) {}

// ****************************************************************************
// * C_Image Member definitions                                               *
// ****************************************************************************

// Init: Allocates space for raster and palette in GDI-compatible structures.
void C_Image::Init(int iWidth, int iHeight, int iBPP) {
	if (Raster) {delete[]Raster;Raster=0;}
	if (pbmi) {delete[]pbmi;pbmi=0;}
	// Standard members setup
	Transparent=-1;
	BytesPerRow = Width = iWidth; Height=iHeight; BPP=iBPP;
	// Animation Extra members setup:
	xPos=xPos=Delay=0;

	if (BPP==24)
		{BytesPerRow*=3; pbmi=(BITMAPINFO*)new char [sizeof(BITMAPINFO)];}
	else
	{
		pbmi=(BITMAPINFO*)
			new char[sizeof(BITMAPINFOHEADER)+(1<<BPP)*sizeof(COLOR)];
		Palette=(COLOR*)((char*)pbmi+sizeof(BITMAPINFOHEADER));
	}

	BytesPerRow += (ALIGN-Width%ALIGN) % ALIGN;	// Align BytesPerRow
	
	Raster = new char [BytesPerRow*Height];

	pbmi->bmiHeader.biSize=sizeof (BITMAPINFOHEADER);
	pbmi->bmiHeader.biWidth=Width;
	pbmi->bmiHeader.biHeight=-Height;			// negative means up-to-bottom 
	pbmi->bmiHeader.biPlanes=1;
	pbmi->bmiHeader.biBitCount=(BPP<8?8:BPP);	// Our raster is byte-aligned
	pbmi->bmiHeader.biCompression=BI_RGB;
	pbmi->bmiHeader.biSizeImage=0;
	pbmi->bmiHeader.biXPelsPerMeter=11811;
	pbmi->bmiHeader.biYPelsPerMeter=11811;
	pbmi->bmiHeader.biClrUsed=0;
	pbmi->bmiHeader.biClrImportant=0;
}

// GDIPaint: Paint the raster image onto a DC
int C_Image::GDIPaint (HDC hdc,int x, int y)
{
	return SetDIBitsToDevice (hdc,x,y,Width,Height,0,0,
								0,Height,(LPVOID)Raster,pbmi,0);
}

// operator=: copies an object's content to another
C_Image& C_Image::operator = (C_Image& rhs)
	{
		Init(rhs.Width,rhs.Height,rhs.BPP);	// respects virtualization
		memcpy (Raster,rhs.Raster,BytesPerRow*Height);
		memcpy ((char*)Palette,(char*)rhs.Palette,(1<<BPP)*sizeof(*Palette));
		return *this;
	}



// ****************************************************************************
// * C_ImageSet Member definitions                                            *
// ****************************************************************************

// AddImage: Adds an image object to the back of the img vector.
void C_ImageSet::AddImage (C_Image* newimage)
{
	C_Image ** pTempImg = new C_Image* [nImages+1];
	int n;
	for (n=0;n<nImages;n++) pTempImg[n]=img[n];	// (pointer copy)
	delete[] img;
	img=pTempImg;
	img[n]=newimage;
	nImages++;
}


// ****************************************************************************
// * C_AnimationWindow Member definitions                                     *
// ****************************************************************************

// WndProc: Window Procedure to be used with C_AnimationWindow Objects
//   it calls member .Message of the adequate C_AnimationWindow object.
LRESULT CALLBACK C_AnimationWindow::WndProc
	(HWND hwndparam, unsigned int message, WPARAM wParam, LPARAM lParam)
{
	C_AnimationWindow * pWnd =0;
	if (message==WM_CREATE)
	{
		LPCREATESTRUCT lpcs;
		lpcs=(LPCREATESTRUCT)lParam;
		SetWindowLong (hwndparam,/*GWL_USERDATA*/ -21,(DWORD)lpcs->lpCreateParams);
		((C_AnimationWindow*)lpcs->lpCreateParams)->m_hWnd = hwndparam;
	}
	if (pWnd = (C_AnimationWindow*) GetWindowLong (hwndparam, /*GWL_USERDATA*/-21))
		return pWnd->Message(message,wParam,lParam);
	else return DefWindowProc (hwndparam,message,wParam,lParam);
}

// fnThread: Thread function in charge of looping animation frames.
DWORD WINAPI C_AnimationWindow::fnThread (LPVOID lpParameter)
{
	C_AnimationWindow* window;
	C_ImageSet* anim;
	window=(C_AnimationWindow*)lpParameter;
	anim=window->pAnimation;
	window->bAnimationPlaying=TRUE;
	while (	anim->nLoops ? window->CurrentLoop < anim->nLoops : true )
	{
		while (1)
		{
			InvalidateRect(window->m_hWnd,NULL,FALSE);

			C_Image* & image = anim->img[window->CurrentImage];
			Sleep (image->Delay?image->Delay:100);

			if (window->CurrentImage < anim->nImages-1)
				++window->CurrentImage;
			else
				{window->CurrentImage=0; break;}
			// CurrentImage must always be valid!

		} 
		++window->CurrentLoop;
	}
	window->Rewind();
	window->bAnimationPlaying=FALSE;
	return 0;
}

// Play: Start/Resume animation loop
void C_AnimationWindow::Play ()
{
	if (!bAnimationPlaying)
		if (pAnimation->nImages > 1)
			hThreadAnim = CreateThread(NULL,0,fnThread,this,0,&dwThreadIdAnim);
}

// Stop: Stop animation loop
void C_AnimationWindow::Stop ()
{
	if (bAnimationPlaying)
	{
		TerminateThread(hThreadAnim,0);
		bAnimationPlaying=FALSE;
	}
}

// Rewind: Reset animation loop to its initial values
void C_AnimationWindow::Rewind ()
{
	CurrentLoop=0;
	CurrentImage=0;
}

// Paint: calls the GDIPaint method of the current image in the loop
void C_AnimationWindow::Paint (HDC hdc)
{
	C_Image* & current = pAnimation->img[CurrentImage];
	current->GDIPaint(hdc,current->xPos,current->yPos);
}


// Create: Wrap of initial windowing tasks:
//  - register the windowclass if required
//  - set object's initial values (including pAnimation)
//  - create a window (initially hidden)
HWND C_AnimationWindow::Create (HWND hwndParent, HMENU id, C_ImageSet * is,
								DWORD dwStyle, char* szClassName)
{
	HINSTANCE hinstance = (HINSTANCE) GetWindowLong(hwndParent,/*GWL_HINSTANCE*/-6);

	// if szClassName is not a registered class, register it:
	WNDCLASS wndclass ={CS_HREDRAW|CS_VREDRAW, WndProc, 0, 0, hinstance,
		0, LoadCursor(NULL,IDC_ARROW), (HBRUSH)(COLOR_WINDOW), 0, szClassName};
	if (! GetClassInfo (hinstance,szClassName,&wndclass))
		if (! RegisterClass (&wndclass)) return 0;

	// set initial values (including pAnimation!)
	pAnimation= is;	
	Rewind();
	if (pAnimation->nImages>0)
		m_hWnd=CreateWindow (szClassName,"",dwStyle,
			0,0,0,0,
			hwndParent,(HMENU)id,
			(HINSTANCE)hinstance,this);
	else m_hWnd=NULL;

	return m_hWnd;
}

// Display: Shows the Window in the specified location (upper-left corner)
BOOL C_AnimationWindow::Display (int x, int y)
{
	BOOL bRet=0;
	if (m_hWnd!=NULL)
	{
		MoveWindow (m_hWnd,x,y,
			pAnimation->FrameWidth,pAnimation->FrameHeight,TRUE);
		bRet= ShowWindow (m_hWnd,SW_SHOW);
		Play();
	}
	return bRet;
}

// Message: Processing function for Windows messages.
// (This is a default behavior, you may derive the class or modify this)
LRESULT C_AnimationWindow::Message (UINT iMsg,WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	switch (iMsg)
	{
	case WM_LBUTTONDOWN:
		if (bAnimationPlaying) Stop(); else Play();
		break;
	case WM_RBUTTONDOWN:
		Rewind();
		InvalidateRect(m_hWnd,NULL,FALSE);
		break;
	case WM_PAINT:
		hdc=BeginPaint (m_hWnd,&ps);
		Paint(hdc);
		EndPaint (m_hWnd,&ps);
		return 0;
	case WM_DESTROY:
		Stop();
		return 0;
	}
	return DefWindowProc (m_hWnd,iMsg,wParam,lParam);
}



// ****************************************************************************
// * FILE FORMAT SUPPORT ROUTINES                                             *
// ****************************************************************************

// ****************************************************************************
// * LoadBMP                                                                  *
// *   Load a BMP File into the C_Image object                                *
// *                        (c) Sept2000, Juan Soulie <jsoulie@cplusplus.com> *
// ****************************************************************************
int C_Image::LoadBMP (char* szFileName)
{
	int n;

	// Open file.
	ifstream bmpfile (szFileName , ios::in | ios::binary | ios::_Nocreate);
	if (! bmpfile.is_open()) { ERRORMSG("File not found"); return 0; }

	// *1* LOAD BITMAP FILE HEADER
	struct BITMAPFILEHEADER {
		unsigned short	bfType; 
		unsigned long	bfSize; 
		unsigned short	bfReserved1; 
		unsigned short	bfReserved2; 
		unsigned long	bfOffBits; 
	} bmfh;
	bmpfile.read ((char*)&bmfh,sizeof (bmfh));

	// Check filetype signature
	if (bmfh.bfType!='MB') { ERRORMSG("Not a valid BMP File"); return 0; }

	// *2* LOAD BITMAP INFO HEADER
	struct BITMAPINFOHEADER {
		unsigned long  biSize;
		         long  biWidth;
		         long  biHeight;
		unsigned short biPlanes;
		unsigned short biBitCount;
		unsigned long  biCompression;
		unsigned long  biSizeImage;
		         long  biXPelsPerMeter;
		         long  biYPelsPerMeter;
		unsigned long  biClrUsed;
		unsigned long  biClrImportant;
	} bmih;
	bmpfile.read ((char*)&bmih,sizeof (bmih));

	// Check for supported Color depths
	if ((bmih.biBitCount!=1) &&
		(bmih.biBitCount!=4) &&
		(bmih.biBitCount!=8) &&
		(bmih.biBitCount!=24))
		{ ERRORMSG("Color depth not supported"); return 0; }

	// Check if file is compressed
	if (bmih.biCompression!=0) 
		{ ERRORMSG("File uses unsupported compression"); return 0; }

	// Set: Allocate memory to contain Data
	Init (bmih.biWidth,
		(bmih.biHeight>0) ? bmih.biHeight: -bmih.biHeight,	// abs
		bmih.biBitCount);

	// *3* IF BPP AREN'T 24, LOAD PALETTE.
	if (BPP!=24)
	{
		for (n=0;n< 1<<BPP;n++)
		{
			Palette[n].b=bmpfile.get();
			Palette[n].g=bmpfile.get();
			Palette[n].r=bmpfile.get();
			bmpfile.get();	// 4th byte of RGBQUAD discarded
		}
	}

	// *4* LOAD RASTER DATA

	// Seek Raster Data in file
	bmpfile.seekg (bmfh.bfOffBits,ios::beg);

	int PixelsPerByte = 8/BPP;	//used only if BPP are less than 8
	int BitMask = (1<<BPP)-1;	//used only if BPP are less than 8

	// Raster Data Rows are 32bit aligned in BMP files.
	int RowAlignmentInFile = ((4- ((Width*BPP+7)/8)%4)%4); // (bytes)

	for (int row=0; row<Height; row++)
	{
		char * pPixel;
		// If height is positive the bmp is bottom-up, set adequate row info:
		pPixel= Raster + BytesPerRow *
			( (bmih.biHeight>0)? Height-row-1 : row );

		if (BPP >= 8)	// 8 or more BPP: Read as block.
			bmpfile.read (pPixel, Width*BPP/8);

		else				// Less than 8BPP: Read and store byte aligned.
		{
			int charGot;
			for (int col=0; col < Width; col+=PixelsPerByte)
			{
				charGot=bmpfile.get();
				for (int bit=8 ; bit >0 ; bit -= BPP)	// high to low
					*pPixel++ = (charGot>> (bit - BPP)) & BitMask;
			}
		}
		// Ignore aligment bytes of file:
		for (int m=0; m<RowAlignmentInFile; m++) bmpfile.get ();
	}

	bmpfile.close();
	return 1;
}


// ****************************************************************************
// * SaveBMP                                                                  *
// *   Save the content of a C_Image object into a BMP file                   *
// *                        (c) Sept2000, Juan Soulie <jsoulie@cplusplus.com> *
// ****************************************************************************
int C_Image::SaveBMP (char * szFileName)
{
	int n;

	// Create file.
	ofstream bmpfile (szFileName , ios::out | ios::binary | ios::trunc);
	if (! bmpfile.is_open()) { ERRORMSG("Error creating file"); return 0;}

	// determine BPP for file:
	int SaveBPP;
	if (BPP == 1) SaveBPP=1;
	else if (BPP <= 4) SaveBPP=4;
	else if (BPP <= 8) SaveBPP=8;
	else SaveBPP=24;

	// *1* SAVE BITMAP FILE HEADER
	struct BITMAPFILEHEADER {
		unsigned short	bfType; 
		unsigned long	bfSize; 
		unsigned short	bfReserved1; 
		unsigned short	bfReserved2; 
		unsigned long	bfOffBits; 
	} bmfh;

	bmfh.bfType='MB';
	bmfh.bfSize=0;	// TO DO
	bmfh.bfReserved1 = bmfh.bfReserved2 = 0;
	bmfh.bfOffBits = 54 + ((SaveBPP==24) ? 0 : (1<<SaveBPP)*4);
	bmpfile.write ((char*)&bmfh,sizeof (bmfh));

	// *2* SAVE BITMAP INFO HEADER
	struct BITMAPINFOHEADER {
		unsigned long  biSize;
		         long  biWidth;
		         long  biHeight;
		unsigned short biPlanes;
		unsigned short biBitCount;
		unsigned long  biCompression;
		unsigned long  biSizeImage;
		         long  biXPelsPerMeter;
		         long  biYPelsPerMeter;
		unsigned long  biClrUsed;
		unsigned long  biClrImportant;
	} bmih;

	bmih.biSize=sizeof(bmih);
	bmih.biWidth=Width;
	bmih.biHeight=Height;	// down-top
	bmih.biPlanes=1;
	bmih.biBitCount=SaveBPP;
	bmih.biCompression=0;// BI_RGB?
	bmih.biSizeImage =(Width*BPP)/8;
	bmih.biSizeImage += (4- (bmih.biSizeImage)%4)%4;
	bmih.biXPelsPerMeter=11811;
	bmih.biYPelsPerMeter=11811;
	bmih.biClrUsed=0;
	bmih.biClrImportant=0;

	bmpfile.write ((char*)&bmih,sizeof (bmih));

	// *3* IF BPP AREN'T 24, SAVE PALETTE.
	if (BPP!=24)
	{
		for (n=0;n< 1<<BPP;n++)
		{
			bmpfile.put(Palette[n].b);
			bmpfile.put(Palette[n].g);
			bmpfile.put(Palette[n].r);
			bmpfile.put((char)0);
		}
		for (;n < 1<<SaveBPP; n++)	// in case SaveBPP is higher than BPP
			bmpfile.write((char*)'\0\0\0\0',4);
	}

	// *4* SAVE RASTER DATA

	int PixelsPerByte = 8/SaveBPP;	//used only if BPP are less than 8
	int BitMask = (1<<SaveBPP)-1;	//used only if BPP are less than 8

	// Raster Data Rows are 32bit aligned in BMP files.
	int RowAlignmentInFile = ((4- ((Width*SaveBPP+7)/8)%4)%4); // (bytes)
	for (int row=0; row<Height; row++)
	{
		char * pPixel;
		// If height is positive the bmp is bottom-up, set adequate row info:
		pPixel= (char*) Raster + BytesPerRow *
			( (bmih.biHeight>0)? Height-row-1 : row );

		if (SaveBPP >= 8)	// 8 or more BPP: Save as block.
			bmpfile.write (pPixel, Width*SaveBPP/8);

		else				// Less than 8BPP: Save packing bytes.
		{
			unsigned char charToPut;
			for (int col=0; col < Width; col+=PixelsPerByte)
			{
				charToPut=0;
				for (int bit=8 ; bit >0 ; bit -= BPP)	// high to low
					charToPut |= (*pPixel++ & BitMask) << (bit - BPP);
				bmpfile.put(charToPut);
			}
		}
		// Ignore aligment bytes of file:
		for (int m=0; m<RowAlignmentInFile; m++) bmpfile.put ((char)0);
	}


	bmpfile.close();
	return 1;

}

// pre-declaration:
int LZWDecoder (char*, char*, short, int, int, int, const int);

// ****************************************************************************
// * LoadGIF                                                                  *
// *   Load a GIF File into the C_ImageSet object                             *
// *                        (c) Nov 2000, Juan Soulie <jsoulie@cplusplus.com> *
// ****************************************************************************
int C_ImageSet::LoadGIF (char * szFileName)
{
	int n;

	// Global GIF variables:
	int GlobalBPP;						// Bits per Pixel.
	COLOR * GlobalColorMap;				// Global colormap (allocate)

	struct GIFGCEtag {				// GRAPHIC CONTROL EXTENSION
		unsigned char BlockSize;		// Block Size: 4 bytes
		unsigned char PackedFields;		// Packed Fields. Bits detail:
										//    0: Transparent Color Flag
										//    1: User Input Flag
										//  2-4: Disposal Method
		unsigned short Delay;			// Delay Time (1/100 seconds)
		unsigned char Transparent;		// Transparent Color Index
	} gifgce;
	int GraphicExtensionFound = 0;

	// OPEN FILE
	ifstream giffile (szFileName,ios::in|ios::_Nocreate|ios::binary);
	if (!giffile.is_open()) {ERRORMSG("File not found");return 0;}

	// *1* READ HEADER (SIGNATURE + VERSION)
	char szSignature[6];				// First 6 bytes (GIF87a or GIF89a)
	giffile.read(szSignature,6);
	if ( memcmp(szSignature,"GIF",2) != 0)
		{ ERRORMSG("Not a GIF File"); return 0; }

	// *2* READ LOGICAL SCREEN DESCRIPTOR
	struct GIFLSDtag {
		unsigned short ScreenWidth;		// Logical Screen Width
		unsigned short ScreenHeight;	// Logical Screen Height
		unsigned char PackedFields;		// Packed Fields. Bits detail:
										//  0-2: Size of Global Color Table
										//    3: Sort Flag
										//  4-6: Color Resolution
										//    7: Global Color Table Flag
		unsigned char Background;		// Background Color Index
		unsigned char PixelAspectRatio;	// Pixel Aspect Ratio
	} giflsd;

	giffile.read((char*)&giflsd,sizeof(giflsd));

	GlobalBPP = (giflsd.PackedFields & 0x07) + 1;

	// fill some animation data:
	FrameWidth = giflsd.ScreenWidth;
	FrameHeight = giflsd.ScreenHeight;
	nLoops = 0;

	// *3* READ/GENERATE GLOBAL COLOR MAP
	GlobalColorMap = new COLOR [1<<GlobalBPP];
	if (giflsd.PackedFields & 0x80)	// File has global color map?
		for (n=0;n< 1<<GlobalBPP;n++)
		{
			GlobalColorMap[n].r=giffile.get();
			GlobalColorMap[n].g=giffile.get();
			GlobalColorMap[n].b=giffile.get();
		}

	else	// GIF standard says to provide an internal default Palette:
		for (n=0;n<256;n++)
			GlobalColorMap[n].r=GlobalColorMap[n].g=GlobalColorMap[n].b=n;

	// *4* NOW WE HAVE 3 POSSIBILITIES:
	//  4a) Get and Extension Block (Blocks with additional information)
	//  4b) Get an Image Separator (Introductor to an image)
	//  4c) Get the trailer Char (End of GIF File)
	do
	{
		int charGot = giffile.get();

		if (charGot == 0x21)		// *A* EXTENSION BLOCK 
		{
			switch (giffile.get())
			{

			case 0xF9:			// Graphic Control Extension

				giffile.read((char*)&gifgce,sizeof(gifgce));
				GraphicExtensionFound++;
				giffile.get(); // Block Terminator (always 0)
				break;

			case 0xFE:			// Comment Extension: Ignored
			case 0x01:			// PlainText Extension: Ignored
			case 0xFF:			// Application Extension: Ignored
			default:			// Unknown Extension: Ignored
				// read (and ignore) data sub-blocks
				while (int nBlockLength = giffile.get())
					for (n=0;n<nBlockLength;n++) giffile.get();
				break;
			}
		}


		else if (charGot == 0x2c) {	// *B* IMAGE (0x2c Image Separator)

			// Create a new Image Object:
			C_Image* NextImage;
			NextImage = new C_Image;

			// Read Image Descriptor
			struct GIFIDtag {	
				unsigned short xPos;			// Image Left Position
				unsigned short yPos;			// Image Top Position
				unsigned short Width;			// Image Width
				unsigned short Height;			// Image Height
				unsigned char PackedFields;		// Packed Fields. Bits detail:
											//  0-2: Size of Local Color Table
											//  3-4: (Reserved)
											//    5: Sort Flag
											//    6: Interlace Flag
											//    7: Local Color Table Flag
			} gifid;

			giffile.read((char*)&gifid, sizeof(gifid));

			int LocalColorMap = (gifid.PackedFields & 0x08)? 1 : 0;

			NextImage->Init (gifid.Width, gifid.Height,
				LocalColorMap ? (gifid.PackedFields&7)+1 : GlobalBPP);

			// Fill NextImage Data
			NextImage->xPos = gifid.xPos;
			NextImage->yPos = gifid.yPos;
			if (GraphicExtensionFound)
			{
				NextImage->Transparent=
					(gifgce.PackedFields&0x01) ? gifgce.Transparent : -1;
				NextImage->Transparency=
					(gifgce.PackedFields&0x1c)>1 ? 1 : 0;
				NextImage->Delay = gifgce.Delay*10;
			}
		
			if (LocalColorMap)		// Read Color Map (if descriptor says so)
				giffile.read((char*)NextImage->Palette,
					sizeof(COLOR)*(1<<NextImage->BPP));

			else					// Otherwise copy Global
				memcpy (NextImage->Palette, GlobalColorMap,
					sizeof(COLOR)*(1<<NextImage->BPP));

			short firstbyte=giffile.get();	// 1st byte of img block (CodeSize)

			// Calculate compressed image block size
				// to fix: this allocates an extra byte per block
			long ImgStart,ImgEnd;				
			ImgEnd = ImgStart = giffile.tellg();
			while (n=giffile.get()) giffile.seekg (ImgEnd+=n+1);
			giffile.seekg (ImgStart);

			// Allocate Space for Compressed Image
			char * pCompressedImage = new char [ImgEnd-ImgStart+4];
  
			// Read and store Compressed Image
			char * pTemp = pCompressedImage;
			while (int nBlockLength = giffile.get())
			{
				giffile.read(pTemp,nBlockLength);
				pTemp+=nBlockLength;
			}

			// Call LZW/GIF decompressor
			n=LZWDecoder(
				(char*) pCompressedImage,
				(char*) NextImage->Raster,
				firstbyte, NextImage->BytesPerRow,//NextImage->AlignedWidth,
				gifid.Width, gifid.Height,
				((gifid.PackedFields & 0x40)?1:0)	//Interlaced?
				);

			if (n)
				AddImage(NextImage);
			else
			{
				delete NextImage;
				ERRORMSG("GIF File Corrupt");
			}

			// Some cleanup
			delete[]pCompressedImage;
			GraphicExtensionFound=0;
		}


		else if (charGot == 0x3b) {	// *C* TRAILER: End of GIF Info
			break; // Ok. Standard End.
		}

	} while (giffile.good());

	giffile.close();
	if (nImages==0) ERRORMSG("Premature End Of File");
	return nImages;
}

// ****************************************************************************
// * LZWDecoder (C/C++)                                                       *
// * Codec to perform LZW (GIF Variant) decompression.                        *
// *                         (c) Nov2000, Juan Soulie <jsoulie@cplusplus.com> *
// ****************************************************************************
//
// Parameter description:
//  - bufIn: Input buffer containing a "de-blocked" GIF/LZW compressed image.
//  - bufOut: Output buffer where result will be stored.
//  - InitCodeSize: Initial CodeSize to be Used
//    (GIF files include this as the first byte in a picture block)
//  - AlignedWidth : Width of a row in memory (including alignment if needed)
//  - Width, Height: Physical dimensions of image.
//  - Interlace: 1 for Interlaced GIFs.
//
int LZWDecoder (char * bufIn, char * bufOut,
				short InitCodeSize, int AlignedWidth,
				int Width, int Height, const int Interlace)
{
	int n;
	int row=0,col=0;				// used to point output if Interlaced
	int nPixels, maxPixels;			// Output pixel counter

	short CodeSize;					// Current CodeSize (size in bits of codes)
	short ClearCode;				// Clear code : resets decompressor
	short EndCode;					// End code : marks end of information

	long whichBit;					// Index of next bit in bufIn
	long LongCode;					// Temp. var. from which Code is retrieved
	short Code;						// Code extracted
	short PrevCode;					// Previous Code
	short OutCode;					// Code to output

	// Translation Table:
	short Prefix[4096];				// Prefix: index of another Code
	unsigned char Suffix[4096];		// Suffix: terminating character
	short FirstEntry;				// Index of first free entry in table
	short NextEntry;				// Index of next free entry in table

	unsigned char OutStack[4097];	// Output buffer
	int OutIndex;					// Characters in OutStack

	int RowOffset;					// Offset in output buffer for current row

	// Set up values that depend on InitCodeSize Parameter.
	CodeSize = InitCodeSize+1;
	ClearCode = (1 << InitCodeSize);
	EndCode = ClearCode + 1;
	NextEntry = FirstEntry = ClearCode + 2;

	whichBit=0;
	nPixels = 0;
	maxPixels = Width*Height;
	RowOffset =0;

	while (nPixels<maxPixels) {
		OutIndex = 0;							// Reset Output Stack

		// GET NEXT CODE FROM bufIn:
		// LZW compression uses code items longer than a single byte.
		// For GIF Files, code sizes are variable between 9 and 12 bits 
		// That's why we must read data (Code) this way:
		LongCode=*((long*)(bufIn+whichBit/8));	// Get some bytes from bufIn
		LongCode>>=(whichBit&7);				// Discard too low bits
		Code =(LongCode & ((1<<CodeSize)-1) );	// Discard too high bits
		whichBit += CodeSize;					// Increase Bit Offset

		// SWITCH, DIFFERENT POSIBILITIES FOR CODE:
		if (Code == EndCode)					// END CODE
			break;								// Exit LZW Decompression loop

		if (Code == ClearCode) {				// CLEAR CODE:
			CodeSize = InitCodeSize+1;			// Reset CodeSize
			NextEntry = FirstEntry;				// Reset Translation Table
			PrevCode=Code;				// Prevent next to be added to table.
			continue;							// restart, to get another code
		}
		if (Code < NextEntry)					// CODE IS IN TABLE
			OutCode = Code;						// Set code to output.

		else {									// CODE IS NOT IN TABLE:
			OutIndex++;			// Keep "first" character of previous output.
			OutCode = PrevCode;					// Set PrevCode to be output
		}

		// EXPAND OutCode IN OutStack
		// - Elements up to FirstEntry are Raw-Codes and are not expanded
		// - Table Prefices contain indexes to other codes
		// - Table Suffices contain the raw codes to be output
		while (OutCode >= FirstEntry) {
			if (OutIndex > 4096) return 0;
			OutStack[OutIndex++] = Suffix[OutCode];	// Add suffix to Output Stack
			OutCode = Prefix[OutCode];				// Loop with preffix
		}

		// NOW OutCode IS A RAW CODE, ADD IT TO OUTPUT STACK.
		if (OutIndex > 4096) return 0;
		OutStack[OutIndex++] = (unsigned char) OutCode;

		// ADD NEW ENTRY TO TABLE (PrevCode + OutCode)
		// (EXCEPT IF PREVIOUS CODE WAS A CLEARCODE)
		if (PrevCode!=ClearCode) {
			Prefix[NextEntry] = PrevCode;
			Suffix[NextEntry] = (unsigned char) OutCode;
			NextEntry++;

			// Prevent Translation table overflow:
			if (NextEntry>=4096) return 0;
      
			// INCREASE CodeSize IF NextEntry IS INVALID WITH CURRENT CodeSize
			if (NextEntry >= (1<<CodeSize)) {
				if (CodeSize < 12) CodeSize++;
				else {}				// Do nothing. Maybe next is Clear Code.
			}
		}

		PrevCode = Code;

		// Avoid the possibility of overflow on 'bufOut'.
		if (nPixels + OutIndex > maxPixels) OutIndex = maxPixels-nPixels;

		// OUTPUT OutStack (LAST-IN FIRST-OUT ORDER)
		for (n=OutIndex-1; n>=0; n--) {
			if (col==Width)						// Check if new row.
			{
				if (Interlace) {				// If interlaced::
					     if ((row&7)==0) {row+=8; if (row>=Height) row=4;}
					else if ((row&3)==0) {row+=8; if (row>=Height) row=2;}
					else if ((row&1)==0) {row+=4; if (row>=Height) row=1;}
					else row+=2;
				}
				else							// If not interlaced:
					row++;

				RowOffset=row*AlignedWidth;		// Set new row offset
				col=0;
			}
			bufOut[RowOffset+col]=OutStack[n];	// Write output
			col++;	nPixels++;					// Increase counters.
		}

	}	// while (main decompressor loop)

	return whichBit;
}

// Refer to WINIMAGE.TXT for copyright and patent notices on GIF and LZW.
