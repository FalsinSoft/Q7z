#include <QFileInfo>
#include "../LZMA/CPP/Common/IntToString.h"
#include "ArchiveUpdateCallback.h"

using namespace NWindows;
using namespace std;

ArchiveUpdateCallback::ArchiveUpdateCallback(const GetFileContentFuncType &getFileContentFunc,
                                             const UpdateInfoFuncType &updateInfoFunc) : m_getFileContentFunc(getFileContentFunc),
                                                                                         m_updateInfoFunc(updateInfoFunc),
                                                                                         m_totalSize(0)
{
}

Z7_COM7F_IMF(ArchiveUpdateCallback::SetTotal(UInt64 size))
{
    m_totalSize = size;
    return S_OK;
}

Z7_COM7F_IMF(ArchiveUpdateCallback::SetCompleted(const UInt64 *completeValue))
{
    m_updateInfoFunc(m_totalSize, *completeValue);
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
    const auto &file = m_fileDataList.at(index);
    NCOM::CPropVariant property = false;

    if(file.requestFileData)
    {
        if(m_getFileContentFunc(file.name, const_cast<QByteArray*>(&file.data)))
        {
            if(file.data.isEmpty()) return E_ABORT;
        }
        *const_cast<bool*>(&file.requestFileData) = false;
    }

    if(file.data.isEmpty())
    {
        NFile::NFind::CFileInfo fileInfo;

        fileInfo.Find(us2fs(file.name.toStdWString().c_str()));
        switch(propID)
        {
            case kpidPath:
                property = excludeBasePath(file.name).toStdWString().c_str();
                break;
            case kpidIsDir:
                property = fileInfo.IsDir();
                break;
            case kpidSize:
                property = fileInfo.Size;
                break;
            case kpidMTime:
                PropVariant_SetFrom_FiTime(property, fileInfo.MTime);
                break;
            case kpidAttrib:
                property = static_cast<UInt32>(fileInfo.GetWinAttrib());
                break;
            case kpidPosixAttrib:
                property = static_cast<UInt32>(fileInfo.GetPosixAttrib());
                break;
        }
    }
    else
    {
        switch(propID)
        {
            case kpidPath:
                property = excludeBasePath(file.name).toStdWString().c_str();
                break;
            case kpidSize:
                property = static_cast<UInt64>(file.data.size());
                break;
            case kpidMTime:
                PropVariant_SetFrom_FiTime(property, CFiTime());
                break;
            case kpidAttrib:
            case kpidPosixAttrib:
                property = static_cast<UInt32>(NULL);
                break;
        }
    }
    property.Detach(value);
    
    return S_OK;
}

Z7_COM7F_IMF(ArchiveUpdateCallback::GetStream(UInt32 index, ISequentialInStream **inStream))
{
    const auto &file = m_fileDataList.at(index);
    const bool isDir = file.data.isEmpty() ? QFileInfo(file.name).isDir() : false;

    if(isDir == false)
    {
        if(file.data.isEmpty())
        {
            CInFileStream *inStreamSpec = new CInFileStream;
            CMyComPtr<ISequentialInStream> inStreamLoc(inStreamSpec);
            if(!inStreamSpec->Open(us2fs(file.name.toStdWString().c_str())))
            {
                return S_FALSE;
            }
            *inStream = inStreamLoc.Detach();
        }
        else
        {
            CInDataStream *inStreamSpec = new CInDataStream(&file.data);
            CMyComPtr<ISequentialInStream> inStreamLoc(inStreamSpec);
            *inStream = inStreamLoc.Detach();
        }
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
    return StringToBstr(m_password.toStdWString().c_str(), password);
}

Z7_COM7F_IMF(ArchiveUpdateCallback::CInDataStream::Read(void *data, UInt32 size, UInt32 *processedSize))
{
    const char *pData = m_data->data();
    quint32 length = size;

    if((m_dataPointer + length) >= m_data->size()) length = (m_data->size() - m_dataPointer);
    memcpy(data, &pData[m_dataPointer], length);
    *processedSize = length;
    m_dataPointer += length;

    return S_OK;
}

Z7_COM7F_IMF(ArchiveUpdateCallback::CInDataStream::Seek(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition))
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

Z7_COM7F_IMF(ArchiveUpdateCallback::CInDataStream::GetSize(UInt64 *size))
{
    *size = m_data->size();
    return S_OK;
}

void ArchiveUpdateCallback::addFile(const QString &name)
{
    m_fileDataList << FileData(name);
}

int ArchiveUpdateCallback::filesCount() const
{
    return m_fileDataList.count();
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
