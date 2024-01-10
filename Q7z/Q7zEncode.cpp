#include <QFile>
#include "LZMA/CPP/7zip/Common/FileStreams.h"
#include "LZMA/CPP/Windows/FileDir.h"
#include "LZMA/CPP/Windows/FileFind.h"
#include "LZMA/CPP/Windows/FileName.h"
#include "Callbacks/ArchiveUpdateCallback.h"
#include "Q7zEncode.h"

extern "C" STDAPI CreateObject(const GUID *clsid, const GUID *iid, void **outObject);

using namespace NWindows;
using namespace NFile;

Q7zEncode::Q7zEncode() : m_compressionLevel(CompressionLevel::Normal),
                         m_compressionMode(CompressionMode::LZMA2),
                         m_mbDictionarySize(16),
                         m_encryptHeaders(false)
{
}

bool Q7zEncode::create(const QString &archiveName, const QList<QString> &filesList, const QString &excludeBasePath)
{
    const GUID CLSIDFormat = { 0x23170F69, 0x40C1, 0x278A, { 0x10, 0x00, 0x00, 0x01, 0x10, 7, 0x00, 0x00 } };
    ArchiveUpdateCallback *updateCallbackSpec = new ArchiveUpdateCallback;
    CMyComPtr<IArchiveUpdateCallback2> updateCallback(updateCallbackSpec);
	COutFileStream *outFileStreamSpec = new COutFileStream;
	CMyComPtr<IOutStream> outFileStream = outFileStreamSpec;
    CMyComPtr<IOutArchive> outArchive;

    for(const auto &file : filesList)
    {
        const FString name(file.toStdString().c_str());
        NFind::CFileInfo fileInfo;
        
        if(!fileInfo.Find(name))
        {
            return false;
        }

        updateCallbackSpec->addFile(file, fileInfo);
    }

    if(CreateObject(&CLSIDFormat, &IID_IOutArchive, reinterpret_cast<void**>(&outArchive)) != S_OK)
    {
        return false;
    }
    if(!outFileStreamSpec->Create(FString(archiveName.toStdString().c_str()), true))
	{
		return false;
	}

    updateCallbackSpec->setPassword(m_password);
    updateCallbackSpec->setExcludeBasePath(excludeBasePath);
    if(!setOutProperties(outArchive))
    {
        QFile::remove(archiveName);
        return false;
    }
    if(outArchive->UpdateItems(outFileStream, updateCallbackSpec->filesCount(), updateCallback) != S_OK)
    {
        QFile::remove(archiveName);
        return false;
    }

	return true;
}

bool Q7zEncode::setOutProperties(IOutArchive *outArchive) const
{
    const wchar_t *propertyNames[] = { L"m", L"x", L"d", L"he" };
    const uint propertyCount = Z7_ARRAY_SIZE(propertyNames);
    NCOM::CPropVariant propertyValues[propertyCount];
    CMyComPtr<ISetProperties> properties;

    outArchive->QueryInterface(IID_ISetProperties, reinterpret_cast<void**>(&properties));
    if(!properties)
    {
        return false;
    }

    propertyValues[0] = parseCompressionMode(m_compressionMode).toStdString().c_str();
    propertyValues[1] = static_cast<UInt32>(m_compressionLevel);
    propertyValues[2] = QString("%1m").arg(m_mbDictionarySize).toStdString().c_str();
    propertyValues[3] = m_encryptHeaders;

    if(properties->SetProperties(propertyNames, propertyValues, propertyCount) != S_OK)
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