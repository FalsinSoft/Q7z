#include "LZMA/CPP/7zip/Common/FileStreams.h"
#include "LZMA/CPP/7zip/Archive/IArchive.h"
#include "LZMA/CPP/Windows/PropVariantConv.h"
#include "Callbacks/ArchiveOpenCallback.h"
#include "Callbacks/ArchiveExtractCallback.h"
#include "Q7zDecode.h"

extern "C" STDAPI CreateObject(const GUID *clsid, const GUID *iid, void **outObject);

using namespace NWindows;
using namespace NFile;

Q7zDecode::Q7zDecode()
{
}

bool Q7zDecode::extract(const QString &archiveName, const QString &outputPath)
{
    const GUID CLSIDFormat = { 0x23170F69, 0x40C1, 0x278A, { 0x10, 0x00, 0x00, 0x01, 0x10, 7, 0x00, 0x00 } };
    ArchiveOpenCallback *openCallbackSpec = new ArchiveOpenCallback;
    CMyComPtr<IArchiveOpenCallback> openCallback(openCallbackSpec);
    ArchiveExtractCallback *extractCallbackSpec = new ArchiveExtractCallback;
    CMyComPtr<IArchiveExtractCallback> extractCallback(extractCallbackSpec);
    CInFileStream *fileSpec = new CInFileStream;
    CMyComPtr<IInStream> file(fileSpec);
    CMyComPtr<IInArchive> archive;
    const UInt64 scanSize = 1 << 23;

    if(CreateObject(&CLSIDFormat, &IID_IInArchive, reinterpret_cast<void**>(&archive)) != S_OK)
    {
        return false;
    }
    if(!fileSpec->Open(FString(archiveName.toStdString().c_str())))
    {
        return false;
    }

    openCallbackSpec->setPassword(m_password);
    if(archive->Open(file, &scanSize, openCallback) != S_OK)
    {
        return false;
    }

    extractCallbackSpec->setArchive(archive);
    extractCallbackSpec->setPassword(m_password);
    extractCallbackSpec->setOutputPath(outputPath);
    if(archive->Extract(NULL, static_cast<UInt32>(-1), false, extractCallback) != S_OK)
    {
        return false;
    }

    return true;
}

bool Q7zDecode::list(const QString &archiveName, FileInfoList *fileList)
{
    const GUID CLSIDFormat = { 0x23170F69, 0x40C1, 0x278A, { 0x10, 0x00, 0x00, 0x01, 0x10, 7, 0x00, 0x00 } };
    ArchiveOpenCallback *openCallbackSpec = new ArchiveOpenCallback;
    CMyComPtr<IArchiveOpenCallback> openCallback(openCallbackSpec);
    CInFileStream *fileSpec = new CInFileStream;
    CMyComPtr<IInStream> file(fileSpec);
    CMyComPtr<IInArchive> archive;
    const UInt64 scanSize = 1 << 23;
    UInt32 itemsCount = 0;

    if(CreateObject(&CLSIDFormat, &IID_IInArchive, reinterpret_cast<void**>(&archive)) != S_OK)
    {
        return false;
    }
    if(!fileSpec->Open(FString(archiveName.toStdString().c_str())))
    {
        return false;
    }

    openCallbackSpec->setPassword(m_password);
    if(archive->Open(file, &scanSize, openCallback) != S_OK)
    {
        return false;
    }

    archive->GetNumberOfItems(&itemsCount);
    for(UInt32 i = 0; i < itemsCount; i++)
    {
        NCOM::CPropVariant property;
        FileInfo fileInfo;
        char buffer[32];

        archive->GetProperty(i, kpidPath, &property);
        fileInfo.name = (property.vt == VT_BSTR) ? QString::fromWCharArray(property.bstrVal) : "????";

        archive->GetProperty(i, kpidSize, &property);
        ConvertPropVariantToShortString(property, buffer);
        fileInfo.size = QString(buffer).toUInt();

        fileList->push_back(fileInfo);
    }

    return true;
}

void Q7zDecode::setPassword(const QString &password)
{
    m_password = password;
}