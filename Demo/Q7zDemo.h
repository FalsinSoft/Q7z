#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_Q7zDemo.h"
#include "Q7zEncode.h"
#include "Q7zDecode.h"

class Q7zDemo : public QMainWindow
{
    Q_OBJECT

    class Encode : public Q7zEncode
    {
    public:
        Encode(Q7zDemo *parent) : m_parent(parent) {}
        bool getFileContent(const QString &name, QByteArray *data) override;
        void encodeInfo(quint64 totalSize, quint64 encodedSize) override;
    private:
        Q7zDemo *m_parent;
    };

    class Decode : public Q7zDecode
    {
    public:
        Decode(Q7zDemo *parent) : m_parent(parent) {}
        bool extractFile(const QString &name, bool *saveToDisk) override;
        void fileContent(const QString &name, const QByteArray &data) override;
        void decodeInfo(quint64 totalSize, quint64 decodedSize) override;
    private:
        Q7zDemo *m_parent;
    };

public:
    Q7zDemo(QWidget *parent = nullptr);
    ~Q7zDemo();

private slots:
    void on_selectArchivePathButton_clicked();
    void on_selectFilesButton_clicked();
    void on_selectExcludeBasePathButton_clicked();
    void on_createArchiveButton_clicked();
    void on_selectOutputPathButton_clicked();
    void on_extractArchiveButton_clicked();
    void on_listArchiveButton_clicked();

private:
    Ui::Q7zDemoClass ui;
};
