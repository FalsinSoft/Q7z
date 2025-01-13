#pragma once

#include <QByteArray>
#include "../LZMA/CPP/7zip/Common/FileStreams.h"

Z7_CLASS_IMP_COM_1(ArchiveMemoryOutStream, IOutStream)
    Z7_IFACE_COM7_IMP(ISequentialOutStream)

public:
    void setData(QByteArray *data);

private:
    QByteArray *m_data = nullptr;
    quint64 m_currentPos = 0;
};