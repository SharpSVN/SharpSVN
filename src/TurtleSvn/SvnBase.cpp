#include "stdafx.h"
#include "SvnBase.h"

#include "AprPool.h"

#include <apr_general.h>

using namespace QQn::Apr;
using namespace QQn::Svn;

static bool _aprInitialized = false;
static int _myLen = 0;

static SvnBase::SvnBase()
{
	if(!_aprInitialized)
	{
		_aprInitialized = true;

		_myLen = strlen("Please initialize CRT");

		apr_initialize();
	}
}

void SvnBase::EnsureLoaded()
{
	GC::KeepAlive(SvnBase::typeid);
}

SvnBase::SvnBase()
{
}

String^ SvnBase::Utf8_PtrToString(const char *ptr)
{
	if(!ptr)
		return nullptr;

	return Utf8_PtrToString(ptr, ::strlen(ptr));
}

String^ SvnBase::Utf8_PtrToString(const char *ptr, int length)
{
	if(!ptr)
		return nullptr;

	return gcnew String(ptr, 0, ::strlen(ptr), System::Text::Encoding::UTF8);
}

const char* SvnBase::Utf8_StringToPtr(String ^value, AprPool^ pool)
{
	if(!value)
		throw gcnew ArgumentNullException("value");
	else if(!pool)
		throw gcnew ArgumentNullException("pool");

	cli::array<unsigned char, 1>^ bytes = System::Text::Encoding::UTF8::get()->GetBytes(value);

	char* pData = (char*)apr_palloc(pool->Handle, bytes->Length+1);

	pin_ptr<unsigned char> pBytes = &bytes[0]; 

	if(pData && pBytes)
		memcpy(pData, pBytes, bytes->Length);

	pData[bytes->Length] = 0;

	return pData;
}

static SvnHandleBase::SvnHandleBase()
{
	SvnBase::EnsureLoaded();
}

SvnHandleBase::SvnHandleBase()
{
}

String^ SvnHandleBase::Utf8_PtrToString(const char *ptr)
{
	return SvnBase::Utf8_PtrToString(ptr);
}

String^ SvnHandleBase::Utf8_PtrToString(const char *ptr, int length)
{
	return SvnBase::Utf8_PtrToString(ptr, length);
}

const char* SvnHandleBase::Utf8_StringToPtr(String ^value, AprPool^ pool)
{
	return SvnBase::Utf8_StringToPtr(value, pool);
}
