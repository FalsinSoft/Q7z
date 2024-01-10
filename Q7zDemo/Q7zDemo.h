#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_Q7zDemo.h"

class Q7zDemo : public QMainWindow
{
    Q_OBJECT

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

private:
    Ui::Q7zDemoClass ui;
};
