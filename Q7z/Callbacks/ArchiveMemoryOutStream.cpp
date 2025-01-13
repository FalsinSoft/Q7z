#include "ArchiveMemoryOutStream.h"

void ArchiveMemoryOutStream::setData(QByteArray *data)
{
    m_data = data;
}

Z7_COM7F_IMF(ArchiveMemoryOutStream::Write(const void *data, UInt32 size, UInt32 *processedSize))
{
    if(m_data == nullptr)  return E_FAIL;

    if((m_currentPos + size) > m_data->size()) m_data->resize(m_currentPos + size);
    memcpy(m_data->data() + m_currentPos, data, size);
    *processedSize = size;
    m_currentPos += size;
    return S_OK;
}

Z7_COM7F_IMF(ArchiveMemoryOutStream::Seek(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition))
{
    if(offset < 0) return HRESULT_WIN32_ERROR_NEGATIVE_SEEK;
    if(m_data == nullptr)  return E_FAIL;

    switch(seekOrigin)
    {
        case STREAM_SEEK_SET:
            m_currentPos = (offset < m_data->size()) ? offset : m_data->size();
            break;
        case STREAM_SEEK_CUR:
            m_currentPos = ((m_currentPos + offset) < m_data->size()) ? (m_currentPos + offset) : m_data->size();
            break;
        case STREAM_SEEK_END:
            m_currentPos = m_data->size();
            break;
        default:
            return STG_E_INVALIDFUNCTION;
    }
    if(newPosition) *newPosition = m_currentPos;

    return S_OK;
}

Z7_COM7F_IMF(ArchiveMemoryOutStream::SetSize(UInt64 newSize))
{
    if(m_data == nullptr)  return E_FAIL;
    m_data->resize(newSize);
    return S_OK;
}
