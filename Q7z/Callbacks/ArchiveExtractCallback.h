#pragma once

#include <QString>
#include "../LZMA/CPP/7zip/Common/FileStreams.h"
#include "../LZMA/CPP/7zip/Archive/IArchive.h"
#include "../LZMA/CPP/7zip/IPassword.h"

class ArchiveExtractCallback Z7_final : public IArchiveExtractCallback,
                                        public ICryptoGetTextPassword,
                                        public CMyUnknownImp
{
    Z7_IFACES_IMP_UNK_2(IArchiveExtractCallback, ICryptoGetTextPassword)
    Z7_IFACE_COM7_IMP(IProgress)
   
public:
    ArchiveExtractCallback();

    void setArchive(IInArchive *archiveHandler);
    void setOutputPath(const QString &outputPath);
    void setPassword(const QString &password);
    Int32 getOperationResult() const;

private:
    CMyComPtr<ISequentialOutStream> m_outFileStream;
    CMyComPtr<IInArchive> m_archiveHandler;
    COutFileStream *m_outFileStreamSpec;
    Int32 m_operationResult;
    UInt32 m_outFileIndex;
    QString m_outputPath;
    QString m_password;

    bool getPropertyDir(UInt32 index, bool *isDir) const;
    bool getPropertyFileFullPath(UInt32 index, UString *fullPath) const;
    bool getPropertyFileAttrib(UInt32 index, UInt32 *attrib) const;
    bool getPropertyFileTime(UInt32 index, CFiTime *fileTime) const;
    bool getPropertyFileSize(UInt32 index, UInt64 *fileSize) const;
};