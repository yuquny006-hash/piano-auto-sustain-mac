//-----------------------------------------------------------------------------
// VST Module Architecture SDK
// Version 1.0    Date : 01/2004
//
// Project      : MIDI Echo Sample
// Filename     : midiecho.cpp
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

//------------------------------------------------------------------------
#define INIT_CLASS_IID
// This macro definition modifies the behavior of DECLARE_CLASS_IID (funknown.h)
// and produces the actual symbols for all interface identifiers.
// It must be defined before including the interface headers and
// in only one source file!
//------------------------------------------------------------------------

#ifndef __midiecho__
#include "midiecho.h"
#endif

//------------------------------------------------------------------------
//  Parameters
//------------------------------------------------------------------------

struct ParamDesc // our private parameter description
{
	const char* name;
	long min;
	long max;
	long value;
};

#define kPPQBase 480 // should be determined via IMasterTrackInfo::getPpqBase

static ParamDesc gParameters[CMidiEcho::kNumParameters] =
{
	{"velocityDecay",	-36,	36,				 -4},
	{"pitchDecay",		-24,	24,				 -1},
	{"repeatRate",		  0,	12,				  5},
	{"quantize",		  1,	kPPQBase/2,		120},
	{"length",			  0,	kPPQBase*4,		120},
	{"echo",			 12,	kPPQBase*2,		120},
	{"echoDecay",		 25,	200,			100},
	{"lengthDecay",		 25,	100,			100}
};

//------------------------------------------------------------------------
//  Editor
//------------------------------------------------------------------------

// The editor XML script is linked as resource
//
// Windows : RC_DATA with id = 1
// MacOS X : midiecho.xml inside the bundle
static CResourceID gEditorResourceID (1, "midiecho.xml");

//------------------------------------------------------------------------
//  CMidiEcho implementation
//------------------------------------------------------------------------

CMidiEcho::CMidiEcho (const char* storageUID)
: CMidiEffect (storageUID),
  velocityDecay (-1),
  pitchDecay (-1),
  repeatRate (-1),
  quantize (-1),
  length (-1),
  echo (-1),
  echoDecay (-1),
  lengthDecay (-1),
  stepStart (0)
{
	// Some configuration...

	flags = kMEIsEditable;		// Yes, we have an editor
	// (set kMEHasInspector if you have an inspector view, too)

	presetsAreChunks (false);	// No, we do not use chunk presets (default)

	setResourceID (gEditorResourceID); // Set editor resource identifier
	// The editor is described in XML, the actual GUI objects (knobs, sliders, etc.)
	// will be created by the host application and communicate with the plugin
	// through its parameter objects.

	// !! Please *DO NOT* allocate any resources or objects in the Ctor,
	// the actual initialization is to be done in the 'initialize'
	// method !!
}

//------------------------------------------------------------------------
tresult CMidiEcho::initialize (FUnknown* context)
{
	// Initialize the base class first...

	tresult result = CMidiEffect::initialize (context);
	if(result != kResultOk)
		return result;

	// ...if it fails, do not go any further.
	// Please note that the host does not call 'terminate' if
	// the initialization of a plugin fails!

	// *) Create parameters:
	// --------------------
	// The base class already provides a parameter container
	// and exposes the parameter objects to the host.
	// All we have to do is, fill the container here:

	for(long tag = 0; tag < kNumParameters; tag++)
	{
		// we only have integer parameter
		ParamDesc* desc = gParameters + tag;
		CParameter* p = params.addIntParam (tag, desc->name, desc->min, desc->max);

		// init the parameter object;
		// this updates the target (this) and initializes our member variables
		// with the correct value
		p->setValue (desc->value);
	}

	// *) Some more initialization:
	// ---------------------------

	// connect our event queues with the accessor provided by
	// the host application
	inQueue.setAccessor (accessor);
	immediateInQueue.setAccessor (accessor);

	// now tell the host that we can play in stop mode, too
	accessor->setCanPlayInStop (true);

	// everything fine and ready to go ;)
	return kResultOk;
}

//------------------------------------------------------------------------
tresult CMidiEcho::terminate ()
{
	// plugin is closing, be sure to release all host interface
	// references here!!

	inQueue.setAccessor (0);
	immediateInQueue.setAccessor (0);

	// base class needs to do some cleanup, too
	return CMidiEffect::terminate ();
}

//------------------------------------------------------------------------
tresult PLUGIN_API CMidiEcho::parameterChanged (IParameter* p, long tag)
{
	// if a parameter object changes (either caused by user
	// manipulating a control in our editor or by track automation),
	// we store its value in a member variable, which will be used
	// in the processing code below (playAction).

	switch(tag)
	{
	case kVelocityDecay	: velocityDecay	= p->getValue (); break;
	case kPitchDecay	: pitchDecay	= p->getValue (); break;
	case kRepeatRate	: repeatRate	= p->getValue (); break;
	case kQuantize		: quantize		= p->getValue (); break;
	case kLength		: length		= p->getValue (); break;
	case kEcho			: echo			= p->getValue (); break;
	case kEchoDecay		: echoDecay		= p->getValue (); break;
	case kLengthDecay	: lengthDecay	= p->getValue (); break;
	}

	return kResultTrue;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CMidiEcho::receiveEvent (IMEObjectID event)
{
	// we received a MIDI event (either from sequencer track or live)...

	// if it is a note, add a copy to our queue
	// as we want to produce an echo later
	if(accessor->getStatus (event) == kNoteOn)
	{
		if(accessor->isImmediateEvent (event))
			immediateInQueue.addEventCopy (event);
		else
			inQueue.addEventCopy (event);
	}

	// NOTE: If you want to "swallow" an incoming event here,
	// just return kResultOk, without passing it back to the host.

	// give event to base class method, which passes
	// it on to our output (e.g. input of next plugin)...
	return CMidiEffect::receiveEvent (event);
}

//------------------------------------------------------------------------
tresult PLUGIN_API CMidiEcho::playAction (long fromPPQ, long toPPQ, bool immediate)
{
	// here is where the magic happens,
	// the sequencer wants us to play a certain time slice...

	CMidiEventQueue& queue = immediate ? immediateInQueue : inQueue;

	IMEObjectID event;
	while((event = queue.retrieveEvent (toPPQ)) != 0)
	{
	    int repeatCounter = 0;
		double scaler = 1.;

		while(repeatCounter++ < repeatRate)
		{
			IMEObjectID e = accessor->duplicateMidiEvent (event);

			long pos = accessor->getStart (event);
			if(isStarted ())
				pos = pos + quantize - ((pos - stepStart) % quantize);
			accessor->setStart (e, pos + (long)((double)(echo * repeatCounter) * scaler));

			int outPitch = accessor->getData1 (e) + pitchDecay * repeatCounter;
			if(outPitch > 0 && outPitch < 128)
				accessor->setData1 (e, outPitch + pitchDecay * repeatCounter);
			else // do not play pitches out of range
			{
				accessor->destroyEvent (e);
				continue;
			}

			int temp = accessor->getData2 (e) + velocityDecay * repeatCounter;
			if(temp <= 0)
				temp = 1;
			else if(temp > 127)
				temp = 127;
			accessor->setData2 (e, temp);

			scaler *= (double)echoDecay / 100.;

			long length = (long)((double)this->length * ((double)lengthDecay / 100.));
			if(length == 0 || accessor->isImmediateEvent (event))
				length = quantize;

			accessor->setLength (e, length);

			// pass event to host output queue
			accessor->passToOutputQueue (e);

			// MIDI event "lifetime" rule:
			// destroy all events which were created inside the Plug-In;
			// otherwise we would produce memory leaks!!
			accessor->destroyEvent (e);
		}

		// see above
		accessor->destroyEvent (event);
	}
	return kResultTrue;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CMidiEcho::positAction (long position)
{
	// sequencer locates to a new position

	stepStart = position;
	return kResultTrue;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CMidiEcho::swapAction (long position, long length)
{
	// a swap action occurs in cycle mode, when jumping back from right
	// to left locator

	// we empty our "live" event queue here...
	immediateInQueue.empty ();
	stepStart = position;
	return kResultTrue;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CMidiEcho::stopAction (long position)
{
	// sequencer has stopped, empty our "live" queue...

	immediateInQueue.empty ();

	// the base class only stores the plaback state
	// in a member variable
	return CMidiEffect::stopAction (position);
}

//------------------------------------------------------------------------
tresult PLUGIN_API CMidiEcho::startAction (long position)
{
	// the sequencer playback has started at a certain position

	// nothing special to do here, call the base class implementation
	return CMidiEffect::startAction (position);
}

