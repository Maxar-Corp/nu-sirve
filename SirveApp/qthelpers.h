#ifndef QTHELPERS_H
#define QTHELPERS_H

#include <qmessagebox.h>
#include <qstring>

class QtHelpers {
    public:
        static void LaunchMessageBox(QString title, QString message);
    private:
};

#endif // QTHELPERS_H