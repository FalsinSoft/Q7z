#include <QFile>
#include "LZMA/CPP/7zip/Common/FileStreams.h"
#include "LZMA/CPP/Windows/FileDir.h"
#include "LZMA/CPP/Windows/FileFind.h"
#include "LZMA/CPP/Windows/FileName.h"
#include "Callbacks/ArchiveUpdateCallback.h"
#include "Callbacks/ArchiveFileOutStream.h"
#include "Callbacks/ArchiveMemoryOutStream.h"
#include "Q7zEncode.h"

STDAPI CreateObject(const GUID *clsid, const GUID *iid, void **outObject);

using namespace NWindows;
using namespace NFile;
using namespace std;

Q7zEncode::Q7zEncode() : m_compressionLevel(CompressionLevel::Normal),
                         m_compressionMode(CompressionMode::LZMA2),
                         m_mbDictionarySize(16),
                         m_encryptHeaders(false)
{
}

bool Q7zEncode::create(const QString &archiveName, const QStringList &files, const QString &excludeBasePath)
{
    ArchiveFileOutStream *fileSpec = new ArchiveFileOutStream;

    if(!fileSpec->create(archiveName))
    {
        return false;
    }

    if(!create(fileSpec, files, excludeBasePath))
    {
        QFile::remove(archiveName);
        return false;
    }

    return true;
}

bool Q7zEncode::create(QByteArray *archiveData, const QStringList &files, const QString &excludeBasePath)
{
    ArchiveMemoryOutStream *fileSpec = new ArchiveMemoryOutStream;

    fileSpec->setData(archiveData);
    return create(fileSpec, files, excludeBasePath);
}

bool Q7zEncode::create(IOutStream *fileSpec, const QStringList &files, const QString &excludeBasePath)
{
    const GUID CLSIDFormat = { 0x23170F69, 0x40C1, 0x278A, { 0x10, 0x00, 0x00, 0x01, 0x10, 7, 0x00, 0x00 } };
    ArchiveUpdateCallback *updateCallbackSpec = new ArchiveUpdateCallback(bind(&Q7zEncode::getFileContent, this, placeholders::_1, placeholders::_2),
                                                                          bind(&Q7zEncode::encodeInfo, this, placeholders::_1, placeholders::_2));
    CMyComPtr<IArchiveUpdateCallback2> updateCallback(updateCallbackSpec);
	CMyComPtr<IOutStream> file = fileSpec;
    CMyComPtr<IOutArchive> archive;

    for(const auto &name : files)
    {
        updateCallbackSpec->addFile(name);
    }

    if(CreateObject(&CLSIDFormat, &IID_IOutArchive, reinterpret_cast<void**>(&archive)) != S_OK)
    {
        return false;
    }

    updateCallbackSpec->setPassword(m_password);
    updateCallbackSpec->setExcludeBasePath(excludeBasePath);
    if(!setOutProperties(archive))
    {
        return false;
    }
    if(archive->UpdateItems(file, updateCallbackSpec->filesCount(), updateCallback) != S_OK)
    {
        return false;
    }

	return true;
}

bool Q7zEncode::setOutProperties(IOutArchive *outArchive) const
{
    CRecordVector<const wchar_t*> propertyNames;
    NCOM::CPropVariant propertyValues[4];
    CMyComPtr<ISetProperties> properties;

    outArchive->QueryInterface(IID_ISetProperties, reinterpret_cast<void**>(&properties));
    if(!properties)
    {
        return false;
    }

    propertyNames.Add(L"m");
    propertyValues[0] = qUtf16Printable(parseCompressionMode(m_compressionMode));
    propertyNames.Add(L"x");
    propertyValues[1] = static_cast<UInt32>(m_compressionLevel);
    propertyNames.Add(L"d");
    propertyValues[2] = qUtf16Printable(QString("%1m").arg(m_mbDictionarySize));
    propertyNames.Add(L"he");
    propertyValues[3] = m_encryptHeaders;

    if(properties->SetProperties(&propertyNames.FrontItem(), propertyValues, 4) != S_OK)
    {
        return false;
    }

    return true;
}

QString Q7zEncode::parseCompressionMode(CompressionMode compressionMode) const
{
    QString value;

    switch(compressionMode)
    {
        default:
        case CompressionMode::LZMA:
            value = "LZMA";
            break;
        case CompressionMode::LZMA2:
            value = "LZMA2";
            break;
        case CompressionMode::PPMd:
            value = "PPMD";
            break;
    }

    return value;
}

void Q7zEncode::setPassword(const QString &password)
{
    m_password = password;
}

void Q7zEncode::setEncryptHeaders(bool encryptHeaders)
{
    m_encryptHeaders = encryptHeaders;
}

void Q7zEncode::setCompressionLevel(CompressionLevel compressionLevel)
{
    m_compressionLevel = compressionLevel;
}

void Q7zEncode::setCompressionMode(CompressionMode compressionMode)
{
    m_compressionMode = compressionMode;
}

void Q7zEncode::setDictionarySize(uint mbDictionarySize)
{
    m_mbDictionarySize = mbDictionarySize;
}

bool Q7zEncode::getFileContent(const QString &name, QByteArray *data)
{
    Q_UNUSED(name);
    Q_UNUSED(data);
    return false;
}

void Q7zEncode::encodeInfo(quint64 totalSize, quint64 encodedSize)
{
    Q_UNUSED(totalSize);
    Q_UNUSED(encodedSize);
}
