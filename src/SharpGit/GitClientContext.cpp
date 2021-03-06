#include <stdafx.h>
#include "GitClientContext.h"

#include "UnmanagedStructs.h"

using namespace SharpGit;
using namespace SharpGit::Implementation;

static bool apr_initialized = false;

static GitPool::GitPool()
{
    if (apr_initialized)
        return;
    apr_initialized = true;

    apr_initialize();

    apr_pool_t* pool = svn_pool_create(nullptr);

    apr_allocator_t* allocator = apr_pool_allocator_get(pool);

    if (allocator)
    {
        apr_allocator_max_free_set(allocator, 1); // Keep a maximum of 1 free block
    }

    svn_utf_initialize2(TRUE, pool);

    git_threads_init();
}

GitPool::!GitPool()
{
    if (_pool)
    {
        try
        {
            svn_pool_destroy(_pool);
        }
        finally
        {
            _pool = nullptr;
        }
    }
}

GitPool::~GitPool()
{
    if (_pool)
    {
        try
        {
            svn_pool_destroy(_pool);
        }
        finally
        {
            _pool = nullptr;
        }
    }
}

char *GitPool::Alloc(size_t sz)
{
    if (!_pool)
        throw gcnew ObjectDisposedException("pool");

    return (char*)apr_pcalloc(_pool, sz);
}

const char *GitPool::AllocDirent(String ^path)
{
    if (!path)
    throw gcnew ArgumentNullException("path");

    if (path->Length >= 1)
    {
    cli::array<unsigned char>^ bytes = System::Text::Encoding::UTF8->GetBytes(path);

    int len = bytes->Length;

    while (len && ((bytes[len-1] == '\\') || bytes[len-1] == '/'))
        len--;

    if (len == 2 && bytes[1] == ':' && path->Length > 2)
        len++;

    char* pData = (char*)Alloc(len+1);

    if (!pData)
        throw gcnew InvalidOperationException();

    pin_ptr<unsigned char> pBytes = &bytes[0];

    if (pData && pBytes)
    {
        memcpy(pData, pBytes, len);

        // Should match: svn_path_internal_style() implementation, but doesn't copy an extra time
        for (int i = 0; i < len; i++)
        if (pData[i] == '\\')
            pData[i] = '/';
    }

    pData[len] = 0;

    if ((len && pData[len-1] == '/') || strstr(pData+1, "//"))
        return svn_dirent_canonicalize(pData, Handle);
    else
    {
        char *pd = pData;
        while (pd = strstr(pd, "/."))
        {
        if (*(pd+2) == '/' || !*(pd+2))
            return svn_dirent_canonicalize(pData, Handle);
        else
            pd++;
        }
    }

    if (pData[0] >= 'a' && pData[0] <= 'z' && pData[1] == ':')
        pData[0] -= ('a' - 'A');

    return pData;
    }
    else
    return "";
}

const char *GitPool::AllocRelpath(String ^path)
{
    if (!path)
    throw gcnew ArgumentNullException("path");

    if (path->Length >= 1)
    {
    cli::array<unsigned char>^ bytes = System::Text::Encoding::UTF8->GetBytes(path);

    int len = bytes->Length;

    while (len && ((bytes[len-1] == '\\') || bytes[len-1] == '/'))
        len--;

    char* pData = (char*)Alloc(len+1);

    if (!pData)
        throw gcnew InvalidOperationException();

    pin_ptr<unsigned char> pBytes = &bytes[0];

    if (pData && pBytes)
    {
        memcpy(pData, pBytes, len);

        // Should match: svn_path_internal_style() implementation, but doesn't copy an extra time
        for (int i = 0; i < len; i++)
        if (pData[i] == '\\')
            pData[i] = '/';
    }

    pData[len] = 0;

    return svn_relpath_canonicalize(pData, Handle);
    }
    else
    return "";
}

const char* GitPool::AllocString(String^ value)
{
    if (!value)
    value = "";

    if (value->Length >= 1)
    {
    cli::array<unsigned char>^ bytes = System::Text::Encoding::UTF8->GetBytes(value);

    char* pData = (char*)Alloc(bytes->Length+1);

    pin_ptr<unsigned char> pBytes = &bytes[0];

    if (pData && pBytes)
        memcpy(pData, pBytes, bytes->Length);

    pData[bytes->Length] = 0;

    return pData;
    }
    else
    return "";
}

git_strarray* GitPool::AllocStringArray(String ^path)
{
    return AllocStringArray(gcnew array<String^> { path });
}

git_strarray* GitPool::AllocStringArray(IEnumerable<String^> ^paths)
{
    git_strarray *pArr = (git_strarray *)apr_pcalloc(Handle, sizeof(*pArr));

    int alloc = 16;
    ICollection<String^>^ coll = dynamic_cast<ICollection<String^>^>(paths);

    if (coll)
        alloc = coll->Count;

    pArr->strings = (char**)apr_pcalloc(Handle,sizeof(const char *)*alloc);

    for each(String ^v in paths)
    {
        if (pArr->count >= alloc)
        {
            int newAlloc = alloc * 2;
            char **old = pArr->strings;
            pArr->strings = (char**)apr_pcalloc(Handle,sizeof(const char *) * newAlloc);
            memcpy(pArr->strings, old, sizeof(const char *) * alloc);
            alloc = newAlloc;
        }

        pArr->strings[pArr->count++] = (char*)AllocString(v);
    }

    return pArr;
}


System::String ^GitBase::Utf8_PtrToString(const char *ptr)
{
    if (! ptr)
        return nullptr;
    else if (! *ptr)
        return "";

    return Utf8_PtrToString(ptr, ::strlen(ptr));
}

System::String ^GitBase::Utf8_PtrToString(const char *ptr, int length)
{
    if (!ptr || length < 0)
        return nullptr;

    if (!*ptr)
        return "";

    return gcnew System::String(ptr, 0, length, System::Text::Encoding::UTF8);
}

System::String ^GitBase::StringFromDirent(const char *dirent, GitPool ^pool)
{
    if (! dirent)
        return nullptr;
    else if (! *dirent)
        return "";

    return Utf8_PtrToString(svn_dirent_local_style(
                                                            svn_dirent_canonicalize(dirent, pool->Handle),
                                                    pool->Handle));
}

System::String ^GitBase::StringFromDirentNoPool(const char *dirent)
{
    if (! dirent)
        return nullptr;
    else if (! *dirent)
        return "";

    GitPool tmp;

    return StringFromDirent(dirent, %tmp);
}

GitId^ GitId::FromPrefix(String ^idPrefix)
{
    if (String::IsNullOrEmpty(idPrefix))
        throw gcnew ArgumentNullException("idPrefix");
    else if (idPrefix->Length > GIT_OID_HEXSZ)
        throw gcnew ArgumentOutOfRangeException("idPrefix");

    String^ add = safe_cast<String^>("0000000000000000000000000000000000000000")->Substring(0, GIT_OID_HEXSZ-idPrefix->Length);

    return gcnew GitId(String::Concat(idPrefix, add));
}
