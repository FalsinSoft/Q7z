#include "LZMA/CPP/7zip/Common/FileStreams.h"
#include "LZMA/CPP/7zip/Archive/IArchive.h"
#include "LZMA/CPP/Windows/PropVariantConv.h"
#include "Callbacks/ArchiveOpenCallback.h"
#include "Callbacks/ArchiveExtractCallback.h"
#include "Q7zDecode.h"

STDAPI CreateObject(const GUID *clsid, const GUID *iid, void **outObject);

using namespace NWindows;
using namespace NFile;
using namespace std;

Q7zDecode::Q7zDecode()
{
}

bool Q7zDecode::extract(const QString &archiveName, const QString &outputPath)
{
    const GUID CLSIDFormat = { 0x23170F69, 0x40C1, 0x278A, { 0x10, 0x00, 0x00, 0x01, 0x10, 7, 0x00, 0x00 } };
    ArchiveOpenCallback *openCallbackSpec = new ArchiveOpenCallback;
    CMyComPtr<IArchiveOpenCallback> openCallback(openCallbackSpec);
    ArchiveExtractCallback *extractCallbackSpec = new ArchiveExtractCallback(bind(&Q7zDecode::extractFile, this, placeholders::_1, placeholders::_2),
                                                                             bind(&Q7zDecode::fileContent, this, placeholders::_1, placeholders::_2));
    CMyComPtr<IArchiveExtractCallback> extractCallback(extractCallbackSpec);
    CInFileStream *fileSpec = new CInFileStream;
    CMyComPtr<IInStream> file(fileSpec);
    CMyComPtr<IInArchive> archive;
    const UInt64 scanSize = 1 << 23;

    if(CreateObject(&CLSIDFormat, &IID_IInArchive, reinterpret_cast<void**>(&archive)) != S_OK)
    {
        return false;
    }
    if(!fileSpec->Open(us2fs(archiveName.toStdWString().c_str())))
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
    if(!fileSpec->Open(us2fs(archiveName.toStdWString().c_str())))
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
        fileInfo.size = QString(buffer).toULongLong();

        fileList->push_back(fileInfo);
    }

    return true;
}

void Q7zDecode::setPassword(const QString &password)
{
    m_password = password;
}

bool Q7zDecode::extractFile(const QString &name, bool *saveToDisk)
{
    Q_UNUSED(name);
    *saveToDisk = true;
    return true;
}

void Q7zDecode::fileContent(const QString &name, const QByteArray &data)
{
    Q_UNUSED(name);
    Q_UNUSED(data);
}
