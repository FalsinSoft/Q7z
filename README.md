# Q7z
Qt library wrapper over LZMA SDK allowing to manage compressed 7z files.

**PLEASE NOTE:** the library has been tested only on Windows and Android

At the moment the library allows you to create an archive in 7z format, read the list of files inside it and extract the files. The options available for creation are basic, including the ability to set a password and also encrypt the list of names inside.

# Basic use
It is possible to use the library to create or extract an archive simply by allocating the respective objects and passing the list of files on disk to insert into the archive and the path where to extract the contained files.

    Q7zEncode encode;
    QStringList files;
    
    encode.setPassword(password);
    encode.setEncryptHeaders(true);
    encode.create(archivePath, files, excludeBasePath);
    
    
    Q7zDecode decode;
    
    decode.setPassword(password);
    decode.extract(archivePath, outputPath);

The *files* parameter and the list of files on disk to insert into the archive. You can exclude any part of the file base path from being stored within the archive.

# Advanced use
The basic use allows you to archive only files present on disk but if you want to add/extract "virtual" files (not physically present on disk but only as data in memory), it is possible to derive the two classes and related methods.

    class Encode : public Q7zEncode
    {
    public:
        bool getFileContent(const QString &name, QByteArray *data) override;
        void encodeInfo(quint64 totalSize, quint64 encodedSize) override;
    };
    
    class Decode : public Q7zDecode
    {
    public:
        bool extractFile(const QString &name, bool *saveToDisk) override;
        void fileContent(const QString &name, const QByteArray &data) override;
        void decodeInfo(quint64 totalSize, quint64 decodedSize) override;
    };

In the list of tile to add to the archive insert your "virtual" file name:

    Encode encode(this);
    QStringList files;
    
    files << "Q7zDemo_readme.txt";

During creation the following method is called:

    bool Q7zDemo::Encode::getFileContent(const QString &name, QByteArray *data)
    {
        if(name == "Q7zDemo_readme.txt")
        {
            data->append("This is an example of virtual file created by code not existing on disk!!!");
            return true;
        }
        return false;
    }

Returning *false* ignores this file which will be considered as a file present on disk. Otherwise you can pass the data to insert into the archive and return *true*.

During the extraction phase the following methods will be called for each file present in the archive:

    bool Q7zDemo::Decode::extractFile(const QString &name, bool *saveToDisk)
    {
        if(name == "Q7zDemo_readme.txt") *saveToDisk = false;
        return true;
    }
    
    void Q7zDemo::Decode::fileContent(const QString &name, const QByteArray &data)
    {
        if(name == "Q7zDemo_readme.txt")
        {
            //...
        }
    }

The first method is called for each file in the archive. if you return *false* the file will be ignored and not extracted. If you return *true* the file will be extracted. Setting the parameter *saveToDisk* to *false* will also call the second method with the data contained in the "virtual" file.

In both operation of encode or decode is always possible to get the current operation status with the following methods:

    void Q7zDemo::Encode::encodeInfo(quint64 totalSize, quint64 encodedSize)
    {
    }
    
    void Q7zDemo::Decode::decodeInfo(quint64 totalSize, quint64 decodedSize)
    {
    }

Param *totalSize* contain the total size of all files to encode/decode and the second param report the current number of bytes just encoded/decoded on real time.