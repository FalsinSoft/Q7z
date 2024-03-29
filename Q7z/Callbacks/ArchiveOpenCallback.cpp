#include "ArchiveOpenCallback.h"

ArchiveOpenCallback::ArchiveOpenCallback()
{
}

Z7_COM7F_IMF(ArchiveOpenCallback::SetTotal(const UInt64 *files, const UInt64 *bytes))
{
    return S_OK;
}

Z7_COM7F_IMF(ArchiveOpenCallback::SetCompleted(const UInt64 *files, const UInt64 *bytes))
{
    return S_OK;
}

Z7_COM7F_IMF(ArchiveOpenCallback::CryptoGetTextPassword(BSTR *password))
{
    return StringToBstr(m_password.toStdWString().c_str(), password);
}

void ArchiveOpenCallback::setPassword(const QString &password)
{
    m_password = password;
}