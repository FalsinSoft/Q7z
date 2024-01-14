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
    bool list(const QString &archiveName, FileInfoList *fileList);

    void setPassword(const QString &password);

protected:
    virtual bool extractFile(const QString &name, bool *saveToDisk);
    virtual void fileContent(const QString &name, const QByteArray &data);

private:
    QString m_password;
};
