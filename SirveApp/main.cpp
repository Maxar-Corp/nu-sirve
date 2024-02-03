//#define _CRTDBG_MAP_ALLOC
#include "SirveApp.h"
#include <QtWidgets/QApplication>

#include <iostream>
#include "logging.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>

void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    QFile file("log/debug_log_file.txt");
    file.open(QIODevice::WriteOnly | QIODevice::Text);

    // Write the message to the file
    QTextStream stream(&file);
    stream << msg << endl;

    // Also output the message to the console
    // QTextStream(stdout) << msg << endl;
}

int main(int argc, char *argv[])
{
    const bool DEBUG_MODE = false;
    if (DEBUG_MODE) {
        qInstallMessageHandler(customMessageHandler);
    }

	QApplication a(argc, argv);
	SirveApp w;
	w.show();
	
	a.exec();

	return 0;
}
