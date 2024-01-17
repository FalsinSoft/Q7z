#include "ArchiveOutStream.h"

bool ArchiveOutStream::create(const QString &fileName)
{
    m_file.setFileName(fileName);
    return m_file.open(QIODeviceBase::WriteOnly | QIODeviceBase::Truncate);
}

void ArchiveOutStream::close()
{
    m_file.close();
}

Z7_COM7F_IMF(ArchiveOutStream::Write(const void *data, UInt32 size, UInt32 *processedSize))
{
    if(!m_file.isOpen()) return E_FAIL;

    *processedSize = m_file.write(reinterpret_cast<const char*>(data), size);
    return S_OK;
}

Z7_COM7F_IMF(ArchiveOutStream::Seek(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition))
{
    qint64 position, size;

    if(offset < 0) return HRESULT_WIN32_ERROR_NEGATIVE_SEEK;
    if(!m_file.isOpen())  return E_FAIL;

    position = m_file.pos();
    size = m_file.size();
    switch(seekOrigin)
    {
        case STREAM_SEEK_SET:
            position = (offset < size) ? offset : size;
            break;
        case STREAM_SEEK_CUR:
            position = ((position + offset) < size) ? (position + offset) : size;
            break;
        case STREAM_SEEK_END:
            position = size;
            break;
        default:
            return STG_E_INVALIDFUNCTION;
    }

    if(m_file.seek(position))
    {
        if(newPosition) *newPosition = position;
        return S_OK;
    }

    return E_FAIL;
}

Z7_COM7F_IMF(ArchiveOutStream::SetSize(UInt64 newSize))
{
    return m_file.resize(newSize) ? S_OK : E_FAIL;
}
