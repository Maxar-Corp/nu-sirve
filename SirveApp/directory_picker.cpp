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

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(lineEdit);
    layout->addWidget(browseButton);

    connect(browseButton, &QPushButton::clicked, this, &DirectoryPicker::openDirectoryPicker);
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
        emit directorySelected(directory);
    }
}

