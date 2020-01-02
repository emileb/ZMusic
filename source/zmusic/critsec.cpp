/*
**
**
**---------------------------------------------------------------------------
** Copyright 2005-2016 Randy Heit
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**---------------------------------------------------------------------------
**
*/
// Brought back for ZMusic because std::mutex under Windows pulls in the
// ENTIRE multithreading library, all combined more than 200kb object code!

#ifdef _WIN32

#ifndef _WINNT_
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

class FInternalCriticalSection
{
public:
	FInternalCriticalSection()
	{
		InitializeCriticalSection(&CritSec);
	}
	~FInternalCriticalSection()
	{
		DeleteCriticalSection(&CritSec);
	}
	void Enter()
	{
		EnterCriticalSection(&CritSec);
	}
	void Leave()
	{
		LeaveCriticalSection(&CritSec);
	}
#if 0
	// SDL has no equivalent functionality, so better not use it on Windows.
	bool TryEnter()
	{
		return TryEnterCriticalSection(&CritSec) != 0;
	}
#endif
private:
	CRITICAL_SECTION CritSec;
};


FInternalCriticalSection *CreateCriticalSection()
{
	return new FInternalCriticalSection();
}

void DeleteCriticalSection(FInternalCriticalSection *c)
{
	delete c;
}

void EnterCriticalSection(FInternalCriticalSection *c)
{
	c->Enter();
}

void LeaveCriticalSection(FInternalCriticalSection *c)
{
	c->Leave();
}

#elif defined __APPLE__

#include "critsec.h"

#include <pthread.h>

class FInternalCriticalSection
{
public:
	FInternalCriticalSection();
	~FInternalCriticalSection();

	void Enter();
	void Leave();

private:
	pthread_mutex_t m_mutex;

};

// TODO: add error handling

FInternalCriticalSection::FInternalCriticalSection()
{
	pthread_mutexattr_t attributes;
	pthread_mutexattr_init(&attributes);
	pthread_mutexattr_settype(&attributes, PTHREAD_MUTEX_RECURSIVE);

	pthread_mutex_init(&m_mutex, &attributes);

	pthread_mutexattr_destroy(&attributes);
}

FInternalCriticalSection::~FInternalCriticalSection()
{
	pthread_mutex_destroy(&m_mutex);
}

void FInternalCriticalSection::Enter()
{
	pthread_mutex_lock(&m_mutex);
}

void FInternalCriticalSection::Leave()
{
	pthread_mutex_unlock(&m_mutex);
}


FInternalCriticalSection *CreateCriticalSection()
{
	return new FInternalCriticalSection();
}

void DeleteCriticalSection(FInternalCriticalSection *c)
{
	delete c;
}

void EnterCriticalSection(FInternalCriticalSection *c)
{
	c->Enter();
}

void LeaveCriticalSection(FInternalCriticalSection *c)
{
	c->Leave();
} 

#else
	
#include "SDL.h"
#include "SDL_thread.h"
#include "i_system.h"

class FInternalCriticalSection
{
public:
	FInternalCriticalSection()
	{
		CritSec = SDL_CreateMutex();
		if (CritSec == NULL)
		{
			I_FatalError("Failed to create a critical section mutex.");
		}
	}
	~FInternalCriticalSection()
	{
		if (CritSec != NULL)
		{
			SDL_DestroyMutex(CritSec);
		}
	}
	void Enter()
	{
		if (SDL_mutexP(CritSec) != 0)
		{
			I_FatalError("Failed entering a critical section.");
		}
	}
	void Leave()
	{
		if (SDL_mutexV(CritSec) != 0)
		{
			I_FatalError("Failed to leave a critical section.");
		}
	}
private:
	SDL_mutex *CritSec;
};

FInternalCriticalSection *CreateCriticalSection()
{
	return new FInternalCriticalSection();
}

void DeleteCriticalSection(FInternalCriticalSection *c)
{
	delete c;
}

void EnterCriticalSection(FInternalCriticalSection *c)
{
	c->Enter();
}

void LeaveCriticalSection(FInternalCriticalSection *c)
{
	c->Leave();
}  
#endif