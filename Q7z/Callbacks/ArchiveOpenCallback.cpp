#include <QScopedPointer>
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
    QScopedPointer<wchar_t, QScopedPointerArrayDeleter<wchar_t>> passwordW;

    passwordW.reset(new wchar_t[m_password.length() + 1]);
    memset(passwordW.data(), 0, (m_password.length() + 1) * sizeof(wchar_t));
    m_password.toWCharArray(passwordW.data());

    return StringToBstr(passwordW.data(), password);
}

void ArchiveOpenCallback::setPassword(const QString &password)
{
    m_password = password;
}