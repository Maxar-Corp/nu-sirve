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

QFrame* QtHelpers::HorizontalLine(QWidget* parent)
{
    QFrame* horizontal_line = new QFrame(parent);
	horizontal_line->setFrameShape(QFrame::HLine);
    return horizontal_line;
}