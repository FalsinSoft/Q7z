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

struct IOutArchive;

class Q7Z_EXPORT Q7zEncode
{
public:
    Q7zEncode();

    enum class CompressionLevel
    {
        None = 0,
        Fastest = 1,
        Fast = 3,
        Normal = 5,
        Maximum = 7,
        Ultra = 9
    };
    enum class CompressionMode
    {
        LZMA2,
        LZMA,
        PPMd
    };

    bool create(const QString &archiveName, const QStringList &files, const QString &excludeBasePath = QString());

    void setPassword(const QString &password);
    void setEncryptHeaders(bool encryptHeaders);
    void setCompressionLevel(CompressionLevel compressionLevel);
    void setCompressionMode(CompressionMode compressionMode);
    void setDictionarySize(uint mbDictionarySize);

protected:
    virtual bool getFileContent(const QString &name, QByteArray *data);
    virtual void encodeInfo(quint64 totalSize, quint64 encodedSize);

private:
    QString m_password;
    bool m_encryptHeaders;
    CompressionLevel m_compressionLevel;
    CompressionMode m_compressionMode;
    uint m_mbDictionarySize;

    bool setOutProperties(IOutArchive *outArchive) const;
    QString parseCompressionMode(CompressionMode compressionMode) const;
};
