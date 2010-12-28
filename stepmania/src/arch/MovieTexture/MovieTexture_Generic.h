#ifndef RAGE_MOVIE_TEXTURE_GENERIC_H
#define RAGE_MOVIE_TEXTURE_GENERIC_H

#include "MovieTexture.h"
#include "RageThreads.h"

class FFMpeg_Helper;
struct RageSurface;

class MovieDecoder
{
public:
	virtual ~MovieDecoder() { }

	virtual RString Open( RString sFile ) = 0;
	virtual void Close() = 0;

	/*
	 * Decode a frame.  Return 1 on success, 0 on EOF, -1 on fatal error.
	 *
	 * If we're lagging behind the video, fTargetTime will be the target
	 * timestamp.  The decoder may skip frames to catch up.  On return,
	 * the current timestamp must be <= fTargetTime.
	 *
	 * Otherwise, fTargetTime will be -1, and the next frame should be
	 * decoded; skip frames only if necessary to recover from errors.
	 */
	virtual int GetFrame( RageSurface *pOut, float fTargetTime ) = 0;

	/* Return the dimensions of the image, in pixels (before aspect ratio
	 * adjustments). */
	virtual int GetWidth() const  = 0;
	virtual int GetHeight() const  = 0;

	// Return the aspect ratio of a pixel in the image.  Usually 1.
	virtual float GetSourceAspectRatio() const { return 1.0f; }

	/* Create a surface acceptable to pass to GetFrame.  This should be
	 * a surface which is realtime-compatible with DISPLAY, and should
	 * attempt to obey bPreferHighColor.  The given size will usually be
	 * the next power of two higher than GetWidth/GetHeight, but on systems
	 * with limited texture resolution, may be smaller. */
	virtual RageSurface *CreateCompatibleSurface( int iTextureWidth, int iTextureHeight, bool bPreferHighColor ) = 0;

	/* The following functions return information about the current frame,
	 * decoded by the last successful call to GetFrame, and will never be
	 * called before that. */

	/* Get the timestamp, in seconds, when the current frame should be
	 * displayed.  The first frame will always be 0. */
	virtual float GetTimestamp() const = 0;

	// Get the duration, in seconds, to display the current frame.
	virtual float GetFrameDuration() const = 0;
};


class MovieTexture_Generic: public RageMovieTexture
{
public:
	MovieTexture_Generic( RageTextureID ID, MovieDecoder *pDecoder );
	virtual ~MovieTexture_Generic();
	RString Init();

	// only called by RageTextureManager::InvalidateTextures
	void Invalidate() { m_uTexHandle = 0; }
	void Update( float fDeltaTime );

	virtual void Reload();

	virtual void SetPosition( float fSeconds );
	virtual void DecodeSeconds( float fSeconds );
	virtual void SetPlaybackRate( float fRate ) { m_fRate = fRate; }
	void SetLooping( bool bLooping=true ) { m_bLoop = bLooping; }
	unsigned GetTexHandle() const { return m_uTexHandle; }

private:
	MovieDecoder *m_pDecoder;

	float m_fRate;
	enum {
		FRAME_NONE, // no frame available; call GetFrame to get one
		FRAME_DECODED, // frame decoded; waiting until it's time to display it
		FRAME_WAITING // frame waiting to be uploaded
	} m_ImageWaiting;
	bool m_bLoop;
	bool m_bWantRewind;
	bool m_bThreaded;

	/*
	 * Only the main thread can change m_State.
	 *
	 * DECODER_QUIT: The decoder thread is not running.  We should only
	 * be in this state internally; when we return after a call, we should
	 * never be in this state.  Start the thread before returning.
	 *
	 * PAUSE_DECODER: The decoder thread is idle.
	 *
	 * PLAYING: The decoder thread is running.
	 */
	enum State { DECODER_QUIT, DECODER_RUNNING } m_State;

	unsigned m_uTexHandle;

	RageSurface *m_pSurface;

	RageSemaphore m_BufferFinished;

	// The time the movie is actually at:
	float m_fClock;
	bool m_bFrameSkipMode;

	static int DecoderThread_start(void *p) { ((MovieTexture_Generic *)(p))->DecoderThread(); return 0; }
	void DecoderThread();
	RageThread m_DecoderThread;

	void UpdateFrame();

	void CreateTexture();
	void DestroyTexture();
	void StartThread();
	void StopThread();

	bool DecodeFrame();
	float CheckFrameTime();
	void DiscardFrame();
};

#endif

/*
 * (c) 2003-2005 Glenn Maynard
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */