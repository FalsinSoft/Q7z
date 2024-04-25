#include <codecvt>
#include <locale>
#include "../LZMA/CPP/Windows/PropVariantConv.h"
#include "../LZMA/CPP/Common/IntToString.h"
#include "../LZMA/CPP/Windows/FileDir.h"
#include "../LZMA/CPP/Windows/FileFind.h"
#include "../LZMA/CPP/Windows/FileName.h"
#include "ArchiveExtractCallback.h"

using namespace NWindows;
using namespace NArchive;
using namespace NFile;
using namespace NDir;
using namespace std;

ArchiveExtractCallback::ArchiveExtractCallback(const ExtractFileFuncType &extractFileFunc,
                                               const FileContentFuncType &fileContentFunc,
                                               const ExtractInfoFuncType &extractInfoFunc) : m_extractFileFunc(extractFileFunc),
                                                                                             m_fileContentFunc(fileContentFunc),
                                                                                             m_extractInfoFunc(extractInfoFunc),
                                                                                             m_operationResult(NExtract::NOperationResult::kOK),
                                                                                             m_totalSize(0)
{
}

Z7_COM7F_IMF(ArchiveExtractCallback::SetTotal(UInt64 size))
{
    m_totalSize = size;
    return S_OK;
}

Z7_COM7F_IMF(ArchiveExtractCallback::SetCompleted(const UInt64 *completeValue))
{
    m_extractInfoFunc(m_totalSize, *completeValue);
    return S_OK;
}

Z7_COM7F_IMF(ArchiveExtractCallback::GetStream(UInt32 index, ISequentialOutStream **outStream, Int32 askExtractMode))
{
    UString fileFullPath, filePath;
    UInt64 fileSize;
    bool isDir;

    *outStream = NULL;
    m_outStream.Release();
    m_fileContentData.clear();

    if(askExtractMode != NExtract::NAskMode::kExtract) return S_OK;
    
    if(!getPropertyFilePath(index, &filePath)
    || !getPropertyFileSize(index, &fileSize)
    || !getPropertyDir(index, &isDir))
    {
        return E_FAIL;
    }

    fileFullPath = fullPath(filePath);
    if(isDir)
    {
        CreateComplexDir(us2fs(fileFullPath));
    }
    else
    {
        wstring_convert<codecvt_utf8<wchar_t>> convert;
        CMyComPtr<ISequentialOutStream> outStreamLoc;

        m_saveFileToDisk = true;
        if(m_extractFileFunc(QString(convert.to_bytes(wstring(filePath.GetBuf())).c_str()), &m_saveFileToDisk) == false)
        {
            return S_OK;
        }

        if(m_saveFileToDisk)
        {
            const int slashPos = fileFullPath.ReverseFind_PathSepar();
            m_outFileStreamSpec = new COutFileStream;
            outStreamLoc = CMyComPtr<ISequentialOutStream>(m_outFileStreamSpec);
            NFind::CFileInfo fileInfo;

            if(fileInfo.Find(us2fs(fileFullPath)))
            {
                if(!DeleteFileAlways(us2fs(fileFullPath)))
                {
                    return E_ABORT;
                }
            }
            else if(slashPos >= 0)
            {
                CreateComplexDir(us2fs(fileFullPath.Left(slashPos)));
            }

            if(!m_outFileStreamSpec->Open(us2fs(fileFullPath), CREATE_ALWAYS))
            {
                return E_ABORT;
            }
        }
        else
        {
            m_fileContentData.resize(fileSize);
            m_fileContentData.fill(0);
            m_outDataStreamSpec = new COutDataStream(&m_fileContentData);
            outStreamLoc = CMyComPtr<ISequentialOutStream>(m_outDataStreamSpec);
        }
        m_outFileIndex = index;
        m_outStream = outStreamLoc;
        *outStream = outStreamLoc.Detach();
    }

    return S_OK;
}

Z7_COM7F_IMF(ArchiveExtractCallback::PrepareOperation(Int32 askExtractMode))
{
    return S_OK;
}

Z7_COM7F_IMF(ArchiveExtractCallback::SetOperationResult(Int32 operationResult))
{
    m_operationResult = operationResult;

    if(operationResult != NExtract::NOperationResult::kOK)
    {
        m_outStream.Release();
        m_fileContentData.clear();
        return E_ABORT;
    }
    
    if(m_outStream)
    {
        UString filePath;

        getPropertyFilePath(m_outFileIndex, &filePath);

        if(m_saveFileToDisk)
        {
            CFiTime fileTime;
            UInt32 attrib;

            if(getPropertyFileTime(m_outFileIndex, &fileTime))
            {
                m_outFileStreamSpec->SetMTime(&fileTime);
            }
            if(getPropertyFileAttrib(m_outFileIndex, &attrib))
            {
                SetFileAttrib_PosixHighDetect(us2fs(fullPath(filePath)), attrib);
            }
            m_outFileStreamSpec->Close();
        }
        else
        {
            wstring_convert<codecvt_utf8<wchar_t>> convert;
            m_fileContentFunc(QString(convert.to_bytes(wstring(filePath.GetBuf())).c_str()), m_fileContentData);
        }
    }
    m_outStream.Release();

    return S_OK;
}

Z7_COM7F_IMF(ArchiveExtractCallback::CryptoGetTextPassword(BSTR *password))
{
    return StringToBstr(m_password.toStdWString().c_str(), password);
}

Z7_COM7F_IMF(ArchiveExtractCallback::COutDataStream::Write(const void *data, UInt32 size, UInt32 *processedSize))
{
    char *pData = m_data->data();
    quint32 length = size;

    if((m_dataPointer + length) >= m_data->size()) length = (m_data->size() - m_dataPointer);
    memcpy(&pData[m_dataPointer], data, length);
    *processedSize = length;
    m_dataPointer += length;

    return S_OK;
}

Z7_COM7F_IMF(ArchiveExtractCallback::COutDataStream::Seek(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition))
{
    if(offset < 0) return HRESULT_WIN32_ERROR_NEGATIVE_SEEK;

    switch(seekOrigin)
    {
        case STREAM_SEEK_SET:
            m_dataPointer = *newPosition = (offset < m_data->size()) ? offset : m_data->size();
            break;
        case STREAM_SEEK_CUR:
            m_dataPointer = *newPosition = ((m_dataPointer + offset) < m_data->size()) ? (m_dataPointer + offset) : m_data->size();
            break;
        case STREAM_SEEK_END:
            m_dataPointer = *newPosition = m_data->size();
            break;
        default:
            return STG_E_INVALIDFUNCTION;
    }

    return S_OK;
}

Z7_COM7F_IMF(ArchiveExtractCallback::COutDataStream::SetSize(UInt64 newSize))
{
    m_data->resize(newSize);
    return S_OK;
}

void ArchiveExtractCallback::setPassword(const QString &password)
{
    m_password = password;
}

void ArchiveExtractCallback::setArchive(IInArchive *archiveHandler)
{
    m_archiveHandler = archiveHandler;
}

void ArchiveExtractCallback::setOutputPath(const QString &outputPath)
{
    m_outputPath = outputPath;
}

bool ArchiveExtractCallback::getPropertyDir(UInt32 index, bool *isDir) const
{
    if(m_archiveHandler)
    {
        NCOM::CPropVariant property;

        if(m_archiveHandler->GetProperty(index, kpidIsDir, &property) != S_OK)
        {
            return false;
        }
        if(property.vt == VT_BOOL)
            *isDir = VARIANT_BOOLToBool(property.boolVal);
        else if(property.vt == VT_EMPTY)
            *isDir = false;
        else
            return false;

        return true;
    }

    return false;
}

bool ArchiveExtractCallback::getPropertyFilePath(UInt32 index, UString *filePath) const
{
    if(m_archiveHandler)
    {
        NCOM::CPropVariant property;

        if(m_archiveHandler->GetProperty(index, kpidPath, &property) != S_OK)
        {
            return false;
        }
        if(property.vt != VT_BSTR)
        {
            return false;
        }
        *filePath = property.bstrVal;

        return true;
    }

    return false;
}

bool ArchiveExtractCallback::getPropertyFileAttrib(UInt32 index, UInt32 *attrib) const
{
    if(m_archiveHandler)
    {
        NCOM::CPropVariant property;

        if(m_archiveHandler->GetProperty(index, kpidAttrib, &property) != S_OK)
        {
            return false;
        }
        *attrib = (property.vt == VT_UI4) ? property.ulVal : 0;

        return true;
    }

    return false;
}

bool ArchiveExtractCallback::getPropertyFileTime(UInt32 index, CFiTime *fileTime) const
{
    if(m_archiveHandler)
    {
        NCOM::CPropVariant property;
        uint prec, ns100;
        FILETIME time;

        if(m_archiveHandler->GetProperty(index, kpidMTime, &property) != S_OK)
        {
            return false;
        }
        time.dwHighDateTime = time.dwLowDateTime = 0;
        prec = ns100 = 0;
        if(property.vt == VT_FILETIME)
        {
            const uint precTemp = property.wReserved1;

            time = property.filetime;
            if(precTemp != 0
            && precTemp <= k_PropVar_TimePrec_1ns
            && property.wReserved3 == 0)
            {
                const uint ns100Temp = property.wReserved2;
                if(ns100Temp < 100)
                {
                    ns100 = ns100Temp;
                    prec = precTemp;
                }
            }
        }

#ifdef _WIN32
        *fileTime = time;
#else
        if(FILETIME_To_timespec(time, *fileTime))
        {
            if((prec == (k_PropVar_TimePrec_Base + 8) || prec == (k_PropVar_TimePrec_Base + 9)) && ns100 != 0)
            {
                fileTime->tv_nsec += ns100;
            }
        }
#endif

        return true;
    }

    return false;
}

bool ArchiveExtractCallback::getPropertyFileSize(UInt32 index, UInt64 *fileSize) const
{
    if(m_archiveHandler)
    {
        NCOM::CPropVariant property;

        if(m_archiveHandler->GetProperty(index, kpidSize, &property) != S_OK)
        {
            return false;
        }
        ConvertPropVariantToUInt64(property, *fileSize);

        return true;
    }

    return false;
}

UString ArchiveExtractCallback::fullPath(const UString &filePath) const
{
    if(!m_outputPath.isEmpty())
    {
        FString outputPath = us2fs(m_outputPath.toStdWString().c_str());
        NName::NormalizeDirPathPrefix(outputPath);
        return UString(outputPath + us2fs(filePath));
    }
    return filePath;
}

Int32 ArchiveExtractCallback::getOperationResult() const
{
    return m_operationResult;
}
