//-----------------------------------------------------------------------------
// VST Module Architecture SDK
// Version 1.0    Date : 01/2004
//
// Project      : MIDI Echo Sample
// Filename     : midiecho.h
// Created by   : Matthias Juwan
// Description  : MIDI Echo Implementation
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

#ifndef __midiecho__
#define __midiecho__

//------------------------------------------------------------------------
// Include SDK C++ Classes
#ifndef __midieffect__
#include "midieffect.h"
#endif

#ifndef __eventqueue__
#include "eventqueue.h"
#endif
//------------------------------------------------------------------------

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//  CMidiEcho class declaration
//------------------------------------------------------------------------
//------------------------------------------------------------------------
class CMidiEcho: public CMidiEffect
{
public:
//------------------------------------------------------------------------
	CMidiEcho (const char* storageUID = 0);

//------------------------------------------------------------------------
	// create function required for plugin factory,
	// it will be called to create new instances of this plugin
	static FUnknown* createInstance (void* context)
	{
		return (IMidiEffect*)new CMidiEcho ((char*)context);
	}

//------------------------------------------------------------------------
	// parameter tags
	enum
	{
		kVelocityDecay,
		kPitchDecay,
		kRepeatRate,
		kQuantize,
		kLength,
		kEcho,
		kEchoDecay,
		kLengthDecay,
		kNumParameters
	};

//------------------------------------------------------------------------
	// CMidiEffect overrides:
	tresult PLUGIN_API initialize (FUnknown* context);
	tresult PLUGIN_API terminate ();
	tresult PLUGIN_API parameterChanged (IParameter*, long tag);
	tresult PLUGIN_API receiveEvent (IMEObjectID event);
	tresult PLUGIN_API playAction (long fromPPQ, long toPPQ, bool immediate);
	tresult PLUGIN_API positAction (long position);
	tresult PLUGIN_API swapAction (long position, long length);
	tresult PLUGIN_API stopAction (long position);
	tresult PLUGIN_API startAction (long position);
//------------------------------------------------------------------------
protected:
	CMidiEventQueue inQueue; // track input queue
	CMidiEventQueue immediateInQueue; // live input queue
	long stepStart;
	long velocityDecay;
	long pitchDecay;
	long repeatRate;
	long quantize;
	long length;
	long echo;
	long echoDecay;
	long lengthDecay;
	long channel;
};

#endif