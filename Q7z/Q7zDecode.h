#pragma once

#include <qglobal.h>
#include <QList>

#ifndef BUILD_STATIC
# if defined(Q7Z_LIB)
#  define Q7Z_EXPORT Q_DECL_EXPORT
# else
#  define Q7Z_EXPORT Q_DECL_IMPORT
# endif
#else
# define Q7Z_EXPORT
#endif

struct IInStream;

class Q7Z_EXPORT Q7zDecode
{
public:
    Q7zDecode();

    struct FileInfo
    {
        QString name;
        quint64 size;
    };
    using FileInfoList = QList<FileInfo>;

    bool extract(const QString &archiveName, const QString &outputPath);
    bool extract(const QByteArray &archiveData);
    bool list(const QString &archiveName, FileInfoList *fileList);
    bool list(const QByteArray &archiveData, FileInfoList *fileList);

    void setPassword(const QString &password);

protected:
    virtual bool extractFile(const QString &name, bool *saveToDisk);
    virtual void fileContent(const QString &name, const QByteArray &data);
    virtual void decodeInfo(quint64 totalSize, quint64 decodedSize);

private:
    QString m_password;

    bool extract(IInStream *fileSpec, const QString &outputPath = QString());
    bool list(IInStream *fileSpec, FileInfoList *fileList);
};
