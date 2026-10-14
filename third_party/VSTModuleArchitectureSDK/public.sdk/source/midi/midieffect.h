//-----------------------------------------------------------------------------
// VST Module Architecture SDK
// Version 1.0    Date : 01/2004
//
// Project      : MIDI Base Classes
// Filename     : midieffect.h
// Created by   : Matthias Juwan
// Description  : MIDI Effect Implementation
//
//-----------------------------------------------------------------------------
// LICENSE
// © 2004, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this
//     software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#ifndef __midieffect__
#define __midieffect__

//-----------------------------------------------------------------------------
// SDK Interface Headers
#ifndef __imidieffect__
#include "midi/imidieffect.h"	// Midi Effect interfaces
#endif

#ifndef __iplugparams__
#include "gui/iplugparams.h"	// Parameter interfaces
#endif

#ifndef __iplugui__
#include "gui/iplugui.h"		// Editor interfaces
#endif

#ifndef __iplugstorage__
#include "base/iplugstorage.h"	// Storage interfaces
#endif
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Include SDK C++ Classes
#ifndef __plugxmlgui__
#include "plugxmlgui.h"			// Resource Loader
#endif

#ifndef __plugparams__
#include "plugparams.h"			// Parameter Container
#endif
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  CMidiEffect : Midi Effect base class
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class CMidiEffect:	public IMidiEffect,
					public IGenericPlugController,
					public IPersistentChunk,
					public IUIFactory2,
					public IViewFactory
{
public:
//-----------------------------------------------------------------------------
	CMidiEffect (const char* storageUID = 0);
	virtual ~CMidiEffect ();

//-----------------------------------------------------------------------------
	// Plugin internal methods:
//-----------------------------------------------------------------------------

	// Configuration (to be done in Ctor)
	void presetsAreChunks (bool state = true)	{ exposeChunks = state; }
	void setResourceID (const CResourceID& id)	{ resourceID = id; }

	// Query effect members
	FUnknown* getHostContext () const			{ return context; }
	CParameter* getParameter (long tag) const	{ return params.getParameter (tag); }

	bool isSend () const						{ return usage == kMEUsageSend; }
	bool isInsert () const						{ return usage == kMEUsageInsert; }
	bool isOffline () const						{ return usage == kMEUsageOffline; }
	bool isStarted () const						{ return started; }
	bool isStopped () const						{ return !started; }

//-----------------------------------------------------------------------------
	// Interface methods:
//-----------------------------------------------------------------------------

	DECLARE_FUNKNOWN_METHODS

	tresult PLUGIN_API initialize (FUnknown* context);
	tresult PLUGIN_API terminate ();

	// Midi Effect
	tresult PLUGIN_API getCaps (MidiEffectCaps*);
	tresult PLUGIN_API configure (long usage);
	tresult PLUGIN_API receiveEvent (IMEObjectID event);
	tresult PLUGIN_API playAction (long fromPPQ, long toPPQ, bool immediate);
	tresult PLUGIN_API positAction (long position);
	tresult PLUGIN_API swapAction (long position, long length);
	tresult PLUGIN_API stopAction (long position);
	tresult PLUGIN_API startAction (long position);

	// Parameters
	tresult PLUGIN_API getParameter (const char* name, IParameter**);
	tresult PLUGIN_API parameterChanged (IParameter*, long tag);
	long    PLUGIN_API countParameters ();
	tresult PLUGIN_API getParameterName (long index, char str[128]);
	tresult PLUGIN_API getParameterIndexed (long index, IParameter**);

	// Storage
	tresult PLUGIN_API getUID (char* uid);
	tresult PLUGIN_API setChunk (char* chunk, long size);
	tresult PLUGIN_API getChunk (char* chunk, long* size);

	// Editor
	tresult PLUGIN_API createController (char* name, IPlugController**);
	tresult PLUGIN_API getViewDescription (char* name, char* templateName, char* xml, long* bufferSize);
	tresult PLUGIN_API getResource (const char* path, char* buffer, long* bufferSize);
	tresult PLUGIN_API createView (const char* name, ViewRect* rect, IPlugView** view);
	tresult PLUGIN_API testCondition (const char* condition);
//-----------------------------------------------------------------------------

protected:
//-----------------------------------------------------------------------------
	long flags;					// effect capabilities		(internal config)
	bool exposeChunks;			// use chunk storage?		(internal config)
	FUID storageUID;			// storage unique id		(internal config)
	CResourceID resourceID;		// editor XML resource		(internal config)
	
	long usage;					// effect usage				(set by host)
	bool started;				// effect started?			(set by host)
	FUnknown* context;			// host context				(set by host)
	IMEAccessor2* accessor;		// midi event accessor		(set by host)

	CPlugParams params;			// parameter container
};

#endif