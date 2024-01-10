#pragma once

#include <QString>
#include "../LZMA/CPP/7zip/Common/FileStreams.h"
#include "../LZMA/CPP/7zip/Archive/IArchive.h"
#include "../LZMA/CPP/7zip/IPassword.h"

class ArchiveOpenCallback Z7_final : public IArchiveOpenCallback,
									 public ICryptoGetTextPassword,
									 public CMyUnknownImp
{
	Z7_IFACES_IMP_UNK_2(IArchiveOpenCallback, ICryptoGetTextPassword)

public:
	ArchiveOpenCallback();

	void setPassword(const QString &password);

private:
	QString m_password;
};