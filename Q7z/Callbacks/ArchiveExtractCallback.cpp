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

ArchiveExtractCallback::ArchiveExtractCallback() : m_operationResult(NExtract::NOperationResult::kOK)
{
}

Z7_COM7F_IMF(ArchiveExtractCallback::SetTotal(UInt64 size))
{
    return S_OK;
}

Z7_COM7F_IMF(ArchiveExtractCallback::SetCompleted(const UInt64 *completeValue))
{
    return S_OK;
}

Z7_COM7F_IMF(ArchiveExtractCallback::GetStream(UInt32 index, ISequentialOutStream **outStream, Int32 askExtractMode))
{
    UString fullPath;
    bool isDir;

    *outStream = NULL;
    m_outFileStream.Release();

    if(askExtractMode != NExtract::NAskMode::kExtract) return S_OK;
    
    if(!getPropertyFileFullPath(index, &fullPath))
    {
        return E_FAIL;
    }
    if(!getPropertyDir(index, &isDir))
    {
        return E_FAIL;
    }

    if(isDir)
    {
        CreateComplexDir(fullPath);
    }
    else
    {
        const int slashPos = fullPath.ReverseFind_PathSepar();
        CMyComPtr<ISequentialOutStream> outStreamLoc;
        NFind::CFileInfo fileInfo;

        if(slashPos >= 0) CreateComplexDir(us2fs(fullPath.Left(slashPos)));

        if(fileInfo.Find(fullPath))
        {
            if(!DeleteFileAlways(fullPath))
            {
                return E_ABORT;
            }
        }

        m_outFileStreamSpec = new COutFileStream;
        outStreamLoc = CMyComPtr<ISequentialOutStream>(m_outFileStreamSpec);
        if(!m_outFileStreamSpec->Open(fullPath, CREATE_ALWAYS))
        {
            return E_ABORT;
        }
        m_outFileIndex = index;
        m_outFileStream = outStreamLoc;
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
        m_outFileStream.Release();
        return E_ABORT;
    }

    if(m_outFileStream)
    {
        CFiTime fileTime;
        UInt32 attrib;

        if(getPropertyFileTime(m_outFileIndex, &fileTime))
        {
            m_outFileStreamSpec->SetMTime(&fileTime);
        }
        if(getPropertyFileAttrib(m_outFileIndex, &attrib))
        {
            UString fullPath;
            getPropertyFileFullPath(m_outFileIndex, &fullPath);
            SetFileAttrib_PosixHighDetect(fullPath, attrib);
        }
        m_outFileStreamSpec->Close();
    }
    m_outFileStream.Release();

    return S_OK;
}


Z7_COM7F_IMF(ArchiveExtractCallback::CryptoGetTextPassword(BSTR *password))
{
    return StringToBstr(UString(m_password.toStdString().c_str()), password);
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

bool ArchiveExtractCallback::getPropertyFileFullPath(UInt32 index, UString *fullPath) const
{
    if(m_archiveHandler)
    {
        NCOM::CPropVariant property;
        FString outputPath;
        UString filePath;

        if(m_archiveHandler->GetProperty(index, kpidPath, &property) != S_OK)
        {
            return false;
        }
        if(property.vt != VT_BSTR)
        {
            return false;
        }
        filePath = property.bstrVal;

        outputPath = FString(m_outputPath.toStdString().c_str());
        NName::NormalizeDirPathPrefix(outputPath);
        *fullPath = (outputPath + us2fs(filePath));

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

Int32 ArchiveExtractCallback::getOperationResult() const
{
    return m_operationResult;
}
