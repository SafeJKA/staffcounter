////////////////////////////////////////////////////////////////////////////
//	Copyright: A. Riazi (3 June 2003)
//
//	Email: a.riazi@misbah3com.com
//
//	This code may be used in compiled form in any way you desire. This
//	file may be redistributed unmodified by any means PROVIDING it is 
//	not sold for profit without the authors written consent, and 
//	providing that this notice and the authors name is included.
//
//	This file is provided 'as is' with no expressed or implied warranty.
//	The author accepts no liability if it causes any damage to your computer.
//
//	Do expect bugs.
//	Please let me know of any bugs/mods/improvements.
//	and I will try to fix/incorporate them into this file.
//	Enjoy!
//
//	Description: CStatic Derived Class for playing Gif Animated files.
//
//  Credit: Most of works done by Juan Soulie <jsoulie@cplusplus.com>
////////////////////////////////////////////////////////////////////////////

#ifndef _GIF_ANIMATION_H_
#define _GIF_ANIMATION_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GifAnimation.h : header file
//

#include "winimage.h"
/////////////////////////////////////////////////////////////////////////////
// CGifAnimation window

class CGifAnimation : public CStatic
{
// Construction
public:
	CGifAnimation();

// Attributes
public:
	C_ImageSet AnimGif;
	
// Operations
public:

protected:
	HANDLE hThreadAnim;			// Thread Handle
	DWORD dwThreadIdAnim;		// Thread Identifier

public:
	C_ImageSet * pAnimation;	// Pointer to Animation Raster Information
	
	int CurrentImage;			// Current Image being displayed
	int CurrentLoop;			// Current Loop in animation
	BOOL bAnimationPlaying;		// TRUE when animation is playing.

	// CLASS FUNCTIONS: (not object functions!)
	static LRESULT CALLBACK WndProc (HWND hwnd,UINT iMsg,WPARAM wParam,LPARAM lParam);
	static DWORD WINAPI fnThread (LPVOID lpParameter);

	// Animation control:
	void Play();				// Starts/Resumes animation
	void Stop();				// Stops(Pauses) animation (without rewinding)
	void Rewind();				// Rewinds animation (without stopping)

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGifAnimation)
	//}}AFX_VIRTUAL

// Implementation
public:
	BOOL IsPlaying();
	int LoadAnimatedGif(LPTSTR FileName);
	virtual ~CGifAnimation();

	// Generated message map functions
protected:
	//{{AFX_MSG(CGifAnimation)
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // _GIF_ANIMATION_H_
