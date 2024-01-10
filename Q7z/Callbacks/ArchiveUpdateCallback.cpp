#include "../LZMA/CPP/Common/IntToString.h"
#include "ArchiveUpdateCallback.h"

using namespace NWindows;

ArchiveUpdateCallback::ArchiveUpdateCallback()
{
}

Z7_COM7F_IMF(ArchiveUpdateCallback::SetTotal(UInt64 size))
{
    return S_OK;
}

Z7_COM7F_IMF(ArchiveUpdateCallback::SetCompleted(const UInt64 *completeValue))
{
    return S_OK;
}

Z7_COM7F_IMF(ArchiveUpdateCallback::GetUpdateItemInfo(UInt32 index, Int32 *newData, Int32 *newProperties, UInt32 *indexInArchive))
{
    if(newData) *newData = BoolToInt(true);
    if(newProperties) *newProperties = BoolToInt(true);
    if(indexInArchive) *indexInArchive = static_cast<UInt32>(-1);
    return S_OK;
}

Z7_COM7F_IMF(ArchiveUpdateCallback::GetProperty(UInt32 index, PROPID propID, PROPVARIANT *value))
{
    NCOM::CPropVariant prop;

    if(propID == kpidIsAnti)
    {
        prop = false;
    }
    else
    {
        const FileInfo &fileInfo = m_fileList.at(index);

        switch(propID)
        {
            case kpidPath:  
                prop = fs2us(FString(excludeBasePath(fileInfo.filePath).toStdString().c_str()));
                break;
            case kpidIsDir:
                prop = fileInfo.IsDir();
                break;
            case kpidSize:
                prop = fileInfo.Size;
                break;
            case kpidCTime:
                PropVariant_SetFrom_FiTime(prop, fileInfo.CTime);
                break;
            case kpidATime:
                PropVariant_SetFrom_FiTime(prop, fileInfo.ATime);
                break;
            case kpidMTime:
                PropVariant_SetFrom_FiTime(prop, fileInfo.MTime);
                break;
            case kpidAttrib:
                prop = static_cast<UInt32>(fileInfo.GetWinAttrib());
                break;
            case kpidPosixAttrib:
                prop = static_cast<UInt32>(fileInfo.GetPosixAttrib());
                break;
        }
    }

    prop.Detach(value);
    return S_OK;
}

Z7_COM7F_IMF(ArchiveUpdateCallback::GetStream(UInt32 index, ISequentialInStream **inStream))
{
    const FileInfo &fileInfo = m_fileList.at(index);

    if(fileInfo.IsDir() == false)
    {
        CInFileStream *inStreamSpec = new CInFileStream;
        CMyComPtr<ISequentialInStream> inStreamLoc(inStreamSpec);

        if(!inStreamSpec->Open(FString(fileInfo.filePath.toStdString().c_str())))
        {
            return S_FALSE;
        }
        *inStream = inStreamLoc.Detach();
    }

    return S_OK;
}

Z7_COM7F_IMF(ArchiveUpdateCallback::SetOperationResult(Int32 operationResult))
{
    return S_OK;
}

Z7_COM7F_IMF(ArchiveUpdateCallback::GetVolumeSize(UInt32 index, UInt64 *size))
{
    return S_FALSE;
}

Z7_COM7F_IMF(ArchiveUpdateCallback::GetVolumeStream(UInt32 index, ISequentialOutStream **volumeStream))
{
    return S_FALSE;
}

Z7_COM7F_IMF(ArchiveUpdateCallback::CryptoGetTextPassword2(Int32 *passwordIsDefined, BSTR *password))
{
    *passwordIsDefined = BoolToInt(!m_password.isEmpty());
    return StringToBstr(UString(m_password.toStdString().c_str()), password);
}

void ArchiveUpdateCallback::addFile(const QString &filePath, const NWindows::NFile::NFind::CFileInfo &fileInfo)
{
    m_fileList << FileInfo(filePath, fileInfo);
}

int ArchiveUpdateCallback::filesCount() const
{
    return m_fileList.count();
}

void ArchiveUpdateCallback::setPassword(const QString &password)
{
    m_password = password;
}

void ArchiveUpdateCallback::setExcludeBasePath(const QString &excludeBasePath)
{
    m_excludeBasePath = excludeBasePath;

    if(!m_excludeBasePath.isEmpty())
    {
        m_excludeBasePath.replace("\\", "/");
        if(m_excludeBasePath.right(1) != "/") m_excludeBasePath += "/";
    }
}

QString ArchiveUpdateCallback::excludeBasePath(const QString &filePath) const
{
    const auto basePathLength = m_excludeBasePath.length();

    if(basePathLength > 0
    && filePath.length() > basePathLength
    && filePath.left(basePathLength) == m_excludeBasePath)
    {
        return filePath.right(filePath.length() - basePathLength);
    }

    return filePath;
}
