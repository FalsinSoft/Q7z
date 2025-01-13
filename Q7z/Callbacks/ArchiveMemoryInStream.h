#pragma once

#include <QByteArray>
#include "../LZMA/CPP/7zip/Archive/IArchive.h"
#include "../LZMA/CPP/Common/MyCom.h"
#include "../LZMA/CPP/7zip/IStream.h"

Z7_class_final(ArchiveMemoryInStream) : public IInStream,
                                        public IStreamGetSize,
                                        public CMyUnknownImp
{
    Z7_COM_UNKNOWN_IMP_2(IInStream, IStreamGetSize)
    Z7_IFACE_COM7_IMP(ISequentialInStream)
    Z7_IFACE_COM7_IMP(IInStream)
    Z7_IFACE_COM7_IMP(IStreamGetSize)

public:
    void setData(QByteArray *data);

private:
    QByteArray *m_data = nullptr;
    quint64 m_currentPos = 0;
};