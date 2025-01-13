#pragma once

#include <QFile>
#include "../LZMA/CPP/7zip/Common/FileStreams.h"

Z7_CLASS_IMP_COM_1(ArchiveFileOutStream, IOutStream)
    Z7_IFACE_COM7_IMP(ISequentialOutStream)
public:
    bool create(const QString &fileName);
    void close();

private:
    QFile m_file;
};