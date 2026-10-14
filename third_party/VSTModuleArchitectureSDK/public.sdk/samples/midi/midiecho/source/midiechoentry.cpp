//-----------------------------------------------------------------------------
// VST Module Architecture SDK
// Version 1.0    Date : 01/2004
//
// Project      : MIDI Echo Sample
// Filename     : midiechoentry.cpp
// Created by   : Matthias Juwan
// Description  : Steinberg Module Entry
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
// Include SDK C++ Classes
#ifndef __pluginfactory__
#include "pluginfactory.h"
#endif
//------------------------------------------------------------------------

#ifndef __midiecho__
#include "midiecho.h"
#endif

//------------------------------------------------------------------------
//  Plug-In factory information
//------------------------------------------------------------------------

static PFactoryInfo factoryInfo =
{
	"Steinberg Media Technologies",		// your company name here
	"http://www.steinberg.de",			// company website...
	"mailto:info@steinberg.de",			// ... and contact (currently not displayed in PlugIn Information)
	PFactoryInfo::kNoFlags				// (default)
};

//------------------------------------------------------------------------
//  exported class
//------------------------------------------------------------------------

static PClassInfo pluginClass =
{
	// !! Please do not forget to generate a new unique identifier for each plugin !!
	// Windows : use the Guidgen tool, which comes with Visual Studio
	// or the FUID::generate method.
	INLINE_UID (0x94e8de5f, 0x92554f53, 0x96fae413, 0x3c935a18),

	PClassInfo::kManyInstances,		// multiple instances of the plugin are allowed
									// (currently ignored, always set to kManyInstances)

	kMidiModuleClass,				// this identifies the plugin as MIDI Effect
	"Midi Echo SDK Sample"			// plugin name visible to the user
};

//------------------------------------------------------------------------
//  Module init/exit
//------------------------------------------------------------------------

// nothing special to do when the library is loaded...
bool InitModule ()   { return true; }
bool DeinitModule () { return true; }

//------------------------------------------------------------------------
//  Steinberg Module Entry
//------------------------------------------------------------------------
// do not forget to include a .def (windows) or .exp (mac) file in your project to export
// this function!

IPluginFactory* PLUGIN_API GetPluginFactory ()
{
	if(!gPluginFactory)
	{
		gPluginFactory = new CPluginFactory (factoryInfo);

		// register all exported classes here:
		gPluginFactory->registerClass (&pluginClass, CMidiEcho::createInstance, pluginClass.cid);
	}
	else
		gPluginFactory->addRef ();

	return gPluginFactory;
}
