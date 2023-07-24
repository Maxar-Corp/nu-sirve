#include "qthelpers.h"

void QtHelpers::LaunchMessageBox(QString title, QString message)
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(title);
    msgBox.setText(message);

    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();
}

int QtHelpers::LaunchYesNoMessageBox(QString title, QString message)
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(title);
    msgBox.setText(message);

    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);

    return msgBox.exec();
}

QFrame* QtHelpers::HorizontalLine(QWidget* parent)
{
    QFrame* horizontal_line = new QFrame(parent);
	horizontal_line->setFrameShape(QFrame::HLine);
    return horizontal_line;
}