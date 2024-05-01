#ifndef DIRECTORYPICKER_H
#define DIRECTORYPICKER_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>

class DirectoryPicker : public QWidget
{
    Q_OBJECT

public:
    explicit DirectoryPicker(QWidget *parent = nullptr);

    QString selectedDirectory() const;

signals:
    void directorySelected(const QString &directory);

private slots:
    void openDirectoryPicker();

private:
    QLineEdit *lineEdit;
    QPushButton *browseButton;
    QString currentDirectory;
};

#endif // DIRECTORYPICKER_H

