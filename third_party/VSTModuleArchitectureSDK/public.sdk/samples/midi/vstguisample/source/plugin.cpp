//-----------------------------------------------------------------------------
// VST Module Architecture SDK
// Version 1.0    Date : 01/2004
//
// Project      : Sample Plug-In using VSTGUI
// Filename     : plugin.cpp
// Created by   : Matthias Juwan
// Description  : Plug-In Implementation
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

#ifndef __plugin__
#include "plugin.h"
#endif

#ifndef __plugineditor__
#include "plugineditor.h"
#endif

//------------------------------------------------------------------------
//  Editor
//------------------------------------------------------------------------

// The main part of our editor is implemented using VSTGUI. However, we
// need a minimum XML description to incorporate the preset view for
// our MIDI Effect (see vstguisample.xml for further details).
// The XML script is linked as resource
//
// Windows : RC_DATA with id = 1
// MacOS X : vstguisample.xml inside the bundle
static CResourceID gEditorResourceID (1, "vstguisample.xml");

//------------------------------------------------------------------------
//  CSamplePlugin implementation
//------------------------------------------------------------------------

CSamplePlugin::CSamplePlugin (const char* storageUID)
: CMidiEffect (storageUID),
  editor (0)
{
	// some configuration...

	flags = kMEIsEditable;		// Yes, we have an editor
	// (set kMEHasInspector if you have an inspector view, too)

	presetsAreChunks (false);	// No, we do not use chunk presets (default)

	setResourceID (gEditorResourceID); // Set editor resource identifier

	// !! Please *DO NOT* allocate any resources or objects in the Ctor,
	// the actual initialization is to be done in the 'initialize'
	// method !!
}

//------------------------------------------------------------------------
tresult PLUGIN_API CSamplePlugin::initialize (FUnknown* context)
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

	params.addIntParam (kParameter1, "param1", 0, 100);

	// we create our VSTGUI editor here...
	editor = new CSamplePluginEditor (this);

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CSamplePlugin::terminate ()
{
	if(editor)
		editor->release ();
	editor = 0;

	// base class needs to do some cleanup, too
	return CMidiEffect::terminate ();
}

//------------------------------------------------------------------------
tresult PLUGIN_API CSamplePlugin::parameterChanged (IParameter*, long tag)
{
	// post update for our editor, maybe parameter was changed from "outside"
	if(editor)
		editor->postUpdate ();

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CSamplePlugin::createView (const char* name, ViewRect* rect, IPlugView** view)
{
	if(editor && !editor->isAttached () && !strcmp (name, "VSTGUIEditor"))
	{
		if(rect)
			editor->setRect (*rect);

		*view = editor;
		editor->addRef (); // because the host will release it later
		return kResultTrue;
	}

	return kResultFalse;
}
