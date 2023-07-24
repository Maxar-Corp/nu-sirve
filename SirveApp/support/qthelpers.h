#ifndef QTHELPERS_H
#define QTHELPERS_H

#include <QMessageBox>
#include <QString>
#include <QFrame>
#include <QWidget>

class QtHelpers {
    public:
        static void LaunchMessageBox(QString title, QString message);
        static int LaunchYesNoMessageBox(QString title, QString message);
        static QFrame* HorizontalLine(QWidget* parent = nullptr);
    private:
};

#endif // QTHELPERS_H