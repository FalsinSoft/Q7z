#pragma once

#include <QList>
#include "../LZMA/CPP/7zip/Common/FileStreams.h"
#include "../LZMA/CPP/Windows/FileDir.h"
#include "../LZMA/CPP/Windows/FileFind.h"
#include "../LZMA/CPP/7zip/Archive/IArchive.h"
#include "../LZMA/CPP/7zip/IPassword.h"

class ArchiveUpdateCallback Z7_final : public IArchiveUpdateCallback2,
                                       public ICryptoGetTextPassword2,
                                       public CMyUnknownImp
{
    Z7_IFACES_IMP_UNK_2(IArchiveUpdateCallback2, ICryptoGetTextPassword2)
    Z7_IFACE_COM7_IMP(IProgress)
    Z7_IFACE_COM7_IMP(IArchiveUpdateCallback)

    Z7_class_final(CInDataStream) : public IInStream,
                                    public IStreamGetSize,
                                    public CMyUnknownImp
    {
        Z7_COM_UNKNOWN_IMP_2(IInStream, IStreamGetSize)
        Z7_IFACE_COM7_IMP(ISequentialInStream)
        Z7_IFACE_COM7_IMP(IInStream)
    public:
        CInDataStream(const QByteArray *data) : m_data(data) {}
        Z7_IFACE_COM7_IMP(IStreamGetSize)
    private:
        const QByteArray *m_data;
        quint64 m_dataPointer = 0;
    };

    struct FileData
    {
        FileData(const QString &name) { this->name = name; }
        QString name;
        QByteArray data;
        bool requestFileData = true;
    };

public:
    using GetFileContentFuncType = std::function<bool(const QString&, QByteArray*)>;
    using UpdateInfoFuncType = std::function<void(quint64, quint64)>;
    ArchiveUpdateCallback(const GetFileContentFuncType &getFileContentFunc,
                          const UpdateInfoFuncType &updateInfoFunc);

    void addFile(const QString &name);
    int filesCount() const;
    void setPassword(const QString &password);
    void setExcludeBasePath(const QString &excludeBasePath);

private:
    GetFileContentFuncType m_getFileContentFunc;
    UpdateInfoFuncType m_updateInfoFunc;
    QList<FileData> m_fileDataList;
    QString m_password;
    QString m_excludeBasePath;
    quint64 m_totalSize;

    QString excludeBasePath(const QString &filePath) const;
};
