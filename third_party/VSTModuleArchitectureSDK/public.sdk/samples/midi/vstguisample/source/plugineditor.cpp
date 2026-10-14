//-----------------------------------------------------------------------------
// VST Module Architecture SDK
// Version 1.0    Date : 01/2004
//
// Project      : Sample Plug-In using VSTGUI
// Filename     : plugineditor.cpp
// Created by   : Matthias Juwan
// Description  : Plug-In Editor using VSTGUI
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

#ifndef __plugineditor__
#include "plugineditor.h"
#endif

#ifndef __plugin__
#include "plugin.h"
#endif

//------------------------------------------------------------------------
// Include SDK C++ Classes
#ifndef __plugparams__
#include "plugparams.h"
#endif
//------------------------------------------------------------------------

//-----------------------------------------------------------------------------
enum
{
	// Bitmap resource identifier
	// Windows : Bitmaps used by VSTGUI must be included as BITMAP resources
	// (not RC_DATA)!!

	kBackgroundBm = 2001,	// the background
	kSliderBm				// slider handle
};

//-----------------------------------------------------------------------------
//  CSamplePluginEditor implementation
//-----------------------------------------------------------------------------

CSamplePluginEditor::CSamplePluginEditor (CSamplePlugin* plugin)
: PluginGUIEditor (0, plugin->getHostContext (), plugin),
  slider1 (0)
{
	// editor size is described in vstguisample.xml
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API CSamplePluginEditor::attached (void* parent)
{
	PluginGUIEditor::attached (parent);

	// create a CFrame
	CRect size (rect.left, rect.top, rect.right, rect.bottom);
	frame = new CFrame (size, parent, this); // frame is deleted later by base class!
	
	// set background bitmap
	CBitmap* bmBack = new CBitmap (kBackgroundBm);
	frame->setBackground (bmBack);

	// create a slider for the test parameter
	CBitmap* bmSlider = new CBitmap (kSliderBm);
	CRect r (0, 0, 150, 11);
	r.offset (76, 69);
	CPoint offset (r.left, r.top);
	slider1 = new CSlider (r, this, kParameter1, r.left, r.right - bmSlider->getWidth (), bmSlider, bmBack, offset);
	frame->addView (slider1);
	
	// (...create some more controls here...)

	bmSlider->forget ();
	bmBack->forget ();
	return kResultOk;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API CSamplePluginEditor::removed ()
{
	slider1 = 0;

	// base class deletes the CFrame object
	return PluginGUIEditor::removed ();
}

//-----------------------------------------------------------------------------
void CSamplePluginEditor::update ()
{
	CSamplePlugin* samplePlug = (CSamplePlugin*)getPlugin ();
	
	// update controls to reflect current parameter state

	CParameter* param = samplePlug->getParameter (kParameter1);
	slider1->setValue (param ? param->getNormalized () : 0.f);
}

//-----------------------------------------------------------------------------
void CSamplePluginEditor::valueChanged (CDrawContext *pContext, CControl *pControl)
{
	if(!pControl)
		return;
	
	CSamplePlugin* samplePlug = (CSamplePlugin*)getPlugin ();

	// a control was manipulated, update the corresponding parameter object...

	CParameter* param = samplePlug->getParameter (pControl->getTag ());
	if(param)
		param->setNormalized (pControl->getValue ());
}
