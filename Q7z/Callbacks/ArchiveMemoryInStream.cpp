#include "ArchiveMemoryInStream.h"

void ArchiveMemoryInStream::setData(QByteArray *data)
{
    m_data = data;
    m_currentPos = 0;
}

Z7_COM7F_IMF(ArchiveMemoryInStream::Read(void *data, UInt32 size, UInt32 *processedSize))
{
    if(m_data == nullptr || m_data->size() == 0)  return E_FAIL;

    *processedSize = (size > (m_data->size() - m_currentPos)) ? (m_data->size() - m_currentPos) : size;
    memcpy(data, m_data->data() + m_currentPos, *processedSize);
    m_currentPos += *processedSize;
    return S_OK;
}

Z7_COM7F_IMF(ArchiveMemoryInStream::Seek(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition))
{
    if(offset < 0) return HRESULT_WIN32_ERROR_NEGATIVE_SEEK;
    if(m_data == nullptr || m_data->size() == 0)  return E_FAIL;

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

Z7_COM7F_IMF(ArchiveMemoryInStream::GetSize(UInt64 *size))
{
    *size = (m_data != nullptr) ? m_data->size() : 0;
    return S_OK;
}
