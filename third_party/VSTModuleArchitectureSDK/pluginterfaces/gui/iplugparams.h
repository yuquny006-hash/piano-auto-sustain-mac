//-----------------------------------------------------------------------------
// VST Module Architecture SDK
// Version 1.0    Date : 01/2004
//
// Category     : SDK Core Interfaces
// Filename     : iplugparams.h
// Created by   : Mario Ewald, Matthias Juwan
// Description  : Parameter Interfaces
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

#ifndef __iplugparams__
#define __iplugparams__

#ifndef __funknown__
#include "../base/funknown.h"
#endif

class IPlugController;

//------------------------------------------------------------------------
//  IParameter interface declaration
//------------------------------------------------------------------------

class IParameter: public FUnknown
{
public:
//------------------------------------------------------------------------
	virtual void PLUGIN_API toString (char str[128]) = 0;
	virtual void PLUGIN_API fromString (const char* str) = 0;
	virtual long PLUGIN_API getValue () = 0;
	virtual void PLUGIN_API setValue (long value) = 0;
	virtual long PLUGIN_API getMinValue () = 0;
	virtual long PLUGIN_API getMaxValue () = 0;
	virtual void PLUGIN_API connect (IPlugController*, long tag) = 0;
//------------------------------------------------------------------------
	static const FUID iid;
};

DECLARE_CLASS_IID (IParameter, 0x0466B780, 0xA08611d5, 0xBECC00A0, 0xC9A636BC)

//------------------------------------------------------------------------
//  IDependencies interface declaration
//------------------------------------------------------------------------

class IDependencies: public FUnknown
{
public:
//------------------------------------------------------------------------
	virtual tresult PLUGIN_API addDependent (FUnknown*) = 0;
	virtual tresult PLUGIN_API removeDependent (FUnknown*) = 0;
//------------------------------------------------------------------------
	static const FUID iid;
};

DECLARE_CLASS_IID (IDependencies, 0x6A1937C8, 0x5B9B4C5F, 0xBAA6082C, 0xEEAB330F)

//------------------------------------------------------------------------
//  IDependencies2 interface declaration
//------------------------------------------------------------------------

class IDependencies2: public IDependencies
{
public:
//------------------------------------------------------------------------
	enum { kDoUpdate = 0x01 };

	virtual tresult PLUGIN_API setUpdateFlag (long mask, bool state) = 0;
//------------------------------------------------------------------------
	static const FUID iid;
};

DECLARE_CLASS_IID (IDependencies2, 0xAF872C18, 0x62314AFF, 0xB5865060, 0x90323835)

//------------------------------------------------------------------------
//  IPlugController interface declaration
//------------------------------------------------------------------------

class IPlugController: public FUnknown
{
public:
//------------------------------------------------------------------------
	virtual tresult PLUGIN_API getParameter (const char* name, IParameter**) = 0;
	virtual tresult PLUGIN_API parameterChanged (IParameter*, long tag) = 0;
//------------------------------------------------------------------------
	static const FUID iid;
};

DECLARE_CLASS_IID (IPlugController, 0x0466B781, 0xA08611d5, 0xBECC00A0, 0xC9A636BC)

//------------------------------------------------------------------------
//  IGenericPlugController interface declaration
//------------------------------------------------------------------------

class IGenericPlugController: public IPlugController
{
public:
//------------------------------------------------------------------------
	virtual long    PLUGIN_API countParameters () = 0;
	virtual tresult PLUGIN_API getParameterName (long index, char str[128]) = 0;
	virtual tresult PLUGIN_API getParameterIndexed (long index, IParameter**) = 0;
//------------------------------------------------------------------------
	static const FUID iid;
};

DECLARE_CLASS_IID (IGenericPlugController, 0x0466B782, 0xA08611d5, 0xBECC00A0, 0xC9A636BC)

#endif
