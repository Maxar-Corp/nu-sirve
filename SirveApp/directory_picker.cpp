#include <QApplication>
#include <QFileDialog>
#include <QWidget>
#include <QDebug>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>

#include "directory_picker.h"

DirectoryPicker::DirectoryPicker(QWidget *parent) : QWidget(parent)
{
    lineEdit = new QLineEdit(this);
    browseButton = new QPushButton("Browse", this);
    chooseButton = new QPushButton("Use this Directory", this);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(lineEdit);
    layout->addWidget(browseButton);
    layout->addWidget(chooseButton);

    connect(browseButton, &QPushButton::clicked, this, &DirectoryPicker::openDirectoryPicker);
    connect(chooseButton, &QPushButton::clicked, this, &DirectoryPicker::commitDirectory);
}

QString DirectoryPicker::selectedDirectory() const
{
    return currentDirectory;
}

void DirectoryPicker::openDirectoryPicker()
{
    QString directory = QFileDialog::getExistingDirectory(this, tr("Select Directory"), QDir::homePath());
    if (!directory.isEmpty())
    {
        lineEdit->setText(directory);
        currentDirectory = directory;
        //emit directorySelected(directory);
    }
}

void DirectoryPicker::commitDirectory()
{
    if (currentDirectory != NULL)
        emit directorySelected(currentDirectory);
}


