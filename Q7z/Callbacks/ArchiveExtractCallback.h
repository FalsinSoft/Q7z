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

    Z7_CLASS_IMP_COM_1(COutDataStream, IOutStream)  
        Z7_IFACE_COM7_IMP(ISequentialOutStream)
    public:
        COutDataStream(QByteArray *data) : m_data(data) {}
    private:
        QByteArray *m_data;
        quint64 m_dataPointer = 0;
    };
   
public:
    using ExtractFileFuncType = std::function<bool(const QString&, bool*)>;
    using FileContentFuncType = std::function<void(const QString&, const QByteArray&)>;
    ArchiveExtractCallback(const ExtractFileFuncType &extractFileFunc,
                           const FileContentFuncType &fileContentFunc);

    void setArchive(IInArchive *archiveHandler);
    void setOutputPath(const QString &outputPath);
    void setPassword(const QString &password);
    Int32 getOperationResult() const;

private:
    ExtractFileFuncType m_extractFileFunc;
    FileContentFuncType m_fileContentFunc;
    CMyComPtr<ISequentialOutStream> m_outStream;
    CMyComPtr<IInArchive> m_archiveHandler;
    COutFileStream *m_outFileStreamSpec;
    COutDataStream *m_outDataStreamSpec;
    QByteArray m_fileContentData;
    Int32 m_operationResult;
    bool m_saveFileToDisk;
    UInt32 m_outFileIndex;
    QString m_outputPath;
    QString m_password;

    bool getPropertyDir(UInt32 index, bool *isDir) const;
    bool getPropertyFilePath(UInt32 index, UString *filePath) const;
    bool getPropertyFileAttrib(UInt32 index, UInt32 *attrib) const;
    bool getPropertyFileTime(UInt32 index, CFiTime *fileTime) const;
    bool getPropertyFileSize(UInt32 index, UInt64 *fileSize) const;
    UString fullPath(const UString &filePath) const;
};