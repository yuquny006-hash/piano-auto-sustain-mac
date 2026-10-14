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
#include "midieffect.h"
#endif

//-----------------------------------------------------------------------------
//  CMidiEffect
//-----------------------------------------------------------------------------

CMidiEffect::CMidiEffect (const char* storageUID)
: flags (0),
  exposeChunks (false),
  storageUID (storageUID),
  usage (0),
  started (false),
  context (0),
  accessor (0)
{
	FUNKNOWN_CTOR
	params.init (this);
}

//-----------------------------------------------------------------------------
CMidiEffect::~CMidiEffect ()
{
	FUNKNOWN_DTOR
}

//-----------------------------------------------------------------------------
IMPLEMENT_REFCOUNT (CMidiEffect)

//-----------------------------------------------------------------------------
tresult PLUGIN_API CMidiEffect::queryInterface (const char* iid, void** obj)
{
	QUERY_INTERFACE (iid, obj, ::FUnknown::iid,				IMidiEffect)
	QUERY_INTERFACE (iid, obj, IPluginBase::iid,			IPluginBase)
	QUERY_INTERFACE (iid, obj, IMidiEffect::iid,			IMidiEffect)
	QUERY_INTERFACE (iid, obj, IPlugController::iid,		IPlugController)
	QUERY_INTERFACE (iid, obj, IGenericPlugController::iid,	IGenericPlugController)
	QUERY_INTERFACE (iid, obj, IUIFactory::iid,				IUIFactory)
	QUERY_INTERFACE (iid, obj, IUIFactory2::iid,			IUIFactory2)
	QUERY_INTERFACE (iid, obj, IViewFactory::iid,			IViewFactory)
	if(exposeChunks)
		QUERY_INTERFACE (iid, obj, IPersistentChunk::iid, IPersistentChunk)

	*obj = 0;
	return kNoInterface;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API CMidiEffect::initialize (FUnknown* _context)
{
	if(!_context)
		return kInvalidArgument;

	if(_context->queryInterface (IMEAccessor2::iid, (void**)&accessor) != kResultOk)
		return kInvalidArgument;

	context = _context;
	context->addRef ();

	CParameter::beginUpdates ();
	return kResultOk;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API CMidiEffect::terminate ()
{
	CParameter::endUpdates ();

	if(accessor)
		accessor->release ();
	accessor = 0;

	if(context)
		context->release ();
	context = 0;
	return kResultOk;
}

//-----------------------------------------------------------------------------
//  Midi Effect
//-----------------------------------------------------------------------------

tresult PLUGIN_API CMidiEffect::getCaps (MidiEffectCaps* c)
{
	if(c && c->_sizeof >= sizeof(MidiEffectCaps))
		c->flags = flags;
	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CMidiEffect::configure (long _usage)
{
	usage = _usage;
	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CMidiEffect::receiveEvent (IMEObjectID event)
{
	if(!accessor)
		return kNotInitialized;

	// default handling is passing events to the output
	return accessor->passToOutput (event);
}

//------------------------------------------------------------------------
tresult PLUGIN_API CMidiEffect::playAction (long fromPPQ, long toPPQ, bool immediate)
{
	return kResultTrue;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CMidiEffect::positAction (long position)
{
	return kResultTrue;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CMidiEffect::swapAction (long position, long length)
{
	return kResultTrue;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CMidiEffect::stopAction (long position)
{
	started = false;
	return kResultTrue;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CMidiEffect::startAction (long position)
{
	started = true;
	return kResultTrue;
}

//-----------------------------------------------------------------------------
//  Parameters
//-----------------------------------------------------------------------------

tresult PLUGIN_API CMidiEffect::getParameter (const char* name, IParameter** p)
{
	*p = params.getParameter (name);
	return *p ? kResultTrue : kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CMidiEffect::parameterChanged (IParameter*, long tag)
{
	return kResultFalse;
}

//------------------------------------------------------------------------
long PLUGIN_API CMidiEffect::countParameters ()
{
	return params.total ();
}

//------------------------------------------------------------------------
tresult PLUGIN_API CMidiEffect::getParameterName (long index, char str[128])
{
	return params.getName (index, str) ? kResultTrue : kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CMidiEffect::getParameterIndexed (long index, IParameter** p)
{
	*p = params.getParameter (index);
	return *p ? kResultTrue : kResultFalse;
}

//------------------------------------------------------------------------
//  Storage
//------------------------------------------------------------------------

tresult PLUGIN_API CMidiEffect::getUID (char* uid)
{
	memcpy (uid, storageUID, 16);
	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CMidiEffect::setChunk (char* chunk, long size)
{
	return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CMidiEffect::getChunk (char* chunk, long* size)
{
	if(size)
		*size = 0;
	return kResultFalse;
}

//------------------------------------------------------------------------
//  Editor
//------------------------------------------------------------------------

tresult PLUGIN_API CMidiEffect::createController (char* name, IPlugController** controller)
{
	if(!strcmp (name, "editor") || !strcmp (name, "inspector"))
	{
		*controller = this;
		addRef ();
		return kResultTrue;
	}
	
	*controller = 0;
	return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CMidiEffect::getViewDescription (char* name, char* templateName, char* xml, long* bufferSize)
{
	if(!strcmp (name, "editor") || !strcmp (name, "inspector"))
	{
		// template name in XML must be "editor" or "inspector"!
		if(templateName)
			strcpy (templateName, name);

		return GetResource (0, resourceID, xml, bufferSize) ? kResultTrue : kResultFalse;
	}

	return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CMidiEffect::getResource (const char* path, char* buffer, long* bufferSize)
{
	CResourceID resID;
	resID.set (path);

	return GetResource (0, resID, buffer, bufferSize) ? kResultTrue : kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CMidiEffect::createView (const char* name, ViewRect* rect, IPlugView** view)
{
	if(view)
		*view = 0;
	return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CMidiEffect::testCondition (const char* test)
{
	return kResultFalse;
}
