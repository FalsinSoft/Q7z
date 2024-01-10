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

    struct FileInfo : public NWindows::NFile::NFind::CFileInfoBase
    {
        FileInfo(const QString &filePath, const NWindows::NFile::NFind::CFileInfo &fileInfo) : CFileInfoBase(fileInfo)
        {
            this->filePath = filePath;
        }
        QString filePath;
    };

public:
    ArchiveUpdateCallback();

    void addFile(const QString &filePath, const NWindows::NFile::NFind::CFileInfo &fileInfo);
    int filesCount() const;
    void setPassword(const QString &password);
    void setExcludeBasePath(const QString &excludeBasePath);

private:
    QList<FileInfo> m_fileList;
    QString m_password;
    QString m_excludeBasePath;

    QString excludeBasePath(const QString &filePath) const;
};
