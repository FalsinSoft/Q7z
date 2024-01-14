#include <QFileDialog>
#include <QMessageBox>
#include "Q7zDemo.h"

Q7zDemo::Q7zDemo(QWidget *parent) : QMainWindow(parent)
{
    ui.setupUi(this);
}

Q7zDemo::~Q7zDemo()
{
}

void Q7zDemo::on_selectArchivePathButton_clicked()
{
    const QString archivePath = QFileDialog::getSaveFileName(this,
                                                             tr("Select archive file"),
                                                             QString(),
                                                             QString(),
                                                             nullptr,
                                                             QFileDialog::DontConfirmOverwrite);

    if(!archivePath.isEmpty())
    {
        ui.archivePath->setText(archivePath);
    }
}

void Q7zDemo::on_selectFilesButton_clicked()
{
    const QStringList files = QFileDialog::getOpenFileNames(this, tr("Select files"));

    if(files.count() > 0)
    {
        ui.filesList->clear();

        for(const auto &file : files)
        {
            ui.filesList->addItem(file);
        }
    }
}

void Q7zDemo::on_selectExcludeBasePathButton_clicked()
{
    const QString excludeBasePath = QFileDialog::getExistingDirectory(this,
                                                                      tr("Select exclude path"),
                                                                      QString(),
                                                                      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if(!excludeBasePath.isEmpty())
    {
        ui.excludeBasePath->setText(excludeBasePath);
    }
}

void Q7zDemo::on_createArchiveButton_clicked()
{
    const QString archivePath = ui.archivePath->text();
    const QString excludeBasePath = ui.excludeBasePath->text();
    QStringList files;
    Encode encode;

    files << "Q7zDemo_readme.txt";
    for(int i = 0; i < ui.filesList->count(); i++) files << ui.filesList->item(i)->text();

    if(archivePath.isEmpty() || files.count() == 0)
    {
        QMessageBox::warning(this,
                             tr("Warning"),
                             tr("Please, insert archive name and files"),
                             QMessageBox::Ok);
        return;
    }
    if(ui.setPassword->isChecked())
    {
        const QString password = ui.password->text();

        if(password.isEmpty())
        {
            QMessageBox::warning(this,
                                 tr("Warning"),
                                 tr("Please, write a valid password"),
                                 QMessageBox::Ok);
            return;
        }
        encode.setPassword(password);
        encode.setEncryptHeaders(true);
    }
    
    if(encode.create(archivePath, files, excludeBasePath))
    {
        QMessageBox::information(this,
                                 tr("Information"),
                                 tr("Archive created successfully"),
                                 QMessageBox::Ok);
    }
    else
    {
        QMessageBox::critical(this,
                              tr("Error"),
                              tr("Archive not created!"),
                              QMessageBox::Ok);
    }
}

void Q7zDemo::on_selectOutputPathButton_clicked()
{
    const QString outputPath = QFileDialog::getExistingDirectory(this,
                                                                 tr("Select output path"),
                                                                 QString(),
                                                                 QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if(!outputPath.isEmpty())
    {
        ui.outputPath->setText(outputPath);
    }
}

void Q7zDemo::on_extractArchiveButton_clicked()
{
    const QString archivePath = ui.archivePath->text();
    const QString outputPath = ui.outputPath->text();
    Decode decode;

    if(archivePath.isEmpty() || outputPath.isEmpty())
    {
        QMessageBox::warning(this,
                             tr("Warning"),
                             tr("Please, insert archive name and outputPath path"),
                             QMessageBox::Ok);
        return;
    }
    if(ui.setPassword->isChecked())
    {
        const QString password = ui.password->text();

        if(password.isEmpty())
        {
            QMessageBox::warning(this,
                                 tr("Warning"),
                                 tr("Please, write a valid password"),
                                 QMessageBox::Ok);
            return;
        }
        decode.setPassword(password);
    }

    if(decode.extract(archivePath, outputPath))
    {
        QMessageBox::information(this,
                                 tr("Information"),
                                 tr("Archive extracted successfully"),
                                 QMessageBox::Ok);
    }
    else
    {
        QMessageBox::critical(this,
                              tr("Error"),
                              tr("Archive not extracted!"),
                              QMessageBox::Ok);
    }
}

void Q7zDemo::on_listArchiveButton_clicked()
{
    const QString archivePath = ui.archivePath->text();
    Q7zDecode::FileInfoList fileList;
    Decode decode;

    if(archivePath.isEmpty())
    {
        QMessageBox::warning(this,
                             tr("Warning"),
                             tr("Please, insert archive name"),
                             QMessageBox::Ok);
        return;
    }
    if(ui.setPassword->isChecked())
    {
        const QString password = ui.password->text();

        if(password.isEmpty())
        {
            QMessageBox::warning(this,
                                 tr("Warning"),
                                 tr("Please, write a valid password"),
                                 QMessageBox::Ok);
            return;
        }
        decode.setPassword(password);
    }

    if(decode.list(archivePath, &fileList))
    {
        ui.archiveFilesList->clear();
        for(const auto &file : fileList) ui.archiveFilesList->addItem(QString("%1 (size: %2)").arg(file.name).arg(file.size));
    }
    else
    {
        QMessageBox::critical(this,
                              tr("Error"),
                              tr("Archive not listed!"),
                              QMessageBox::Ok);
    }
}

bool Q7zDemo::Encode::getFileContent(const QString &name, QByteArray *data)
{
    if(name == "Q7zDemo_readme.txt")
    {
        data->append("This is an example of virtual file created by code not existing on disk!!!");
        return true;
    }
    return false;
}

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
