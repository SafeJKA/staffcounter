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

// GifAnimation.cpp : implementation file
//

#include "stdafx.h"
#include "GifAnimation.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGifAnimation

CGifAnimation::CGifAnimation()
{
	pAnimation=NULL;
	CurrentLoop=0;
	CurrentImage=0;
	bAnimationPlaying=false;
}

CGifAnimation::~CGifAnimation()
{
}


BEGIN_MESSAGE_MAP(CGifAnimation, CStatic)
	//{{AFX_MSG_MAP(CGifAnimation)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGifAnimation message handlers
int CGifAnimation::LoadAnimatedGif(LPTSTR FileName)
{
	int Result=AnimGif.LoadGIF(FileName);
	pAnimation=&AnimGif;

	return Result;
}

void CGifAnimation::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
		if (pAnimation->img)
		{
			C_Image* & current = pAnimation->img[CurrentImage/*i*/];		
			current->GDIPaint(dc,current->xPos,current->yPos);
		}
}

// fnThread: Thread function in charge of looping animation frames.
DWORD WINAPI CGifAnimation::fnThread (LPVOID lpParameter)
{
	CGifAnimation* window;
	C_ImageSet* anim;
	window=(CGifAnimation*) lpParameter;
	anim=window->pAnimation;
	window->bAnimationPlaying=TRUE;
	while (	anim->nLoops ? window->CurrentLoop < anim->nLoops : true )
	{
		while (1)
		{
			::InvalidateRect(window->m_hWnd,NULL,FALSE);

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
void CGifAnimation::Play ()
{
	if (!bAnimationPlaying)
		if (pAnimation->nImages > 1)
			hThreadAnim = CreateThread(NULL,0,fnThread,this,0,&dwThreadIdAnim);
}

// Stop: Stop animation loop
void CGifAnimation::Stop ()
{
	if (bAnimationPlaying)
	{
		TerminateThread(hThreadAnim,0);
		bAnimationPlaying=FALSE;
	}
}

// Rewind: Reset animation loop to its initial values
void CGifAnimation::Rewind ()
{
	CurrentLoop=0;
	CurrentImage=0;
}

BOOL CGifAnimation::IsPlaying()
{
	return bAnimationPlaying;
}

