//#define _CRTDBG_MAP_ALLOC
#include "SirveApp.h"
#include <QtWidgets/QApplication>

#include <iostream>
#include "logging.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>

void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    QString LOG_FILE_PATH = "log/debug_log_file.txt";
    QFile file(LOG_FILE_PATH);
    file.open(QIODevice::Append | QIODevice::Text);

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
        QString LOG_FILE_PATH = "log/debug_log_file.txt";

        //Create the intermediate directories if they don't exist
        QDir().mkpath(QFileInfo(LOG_FILE_PATH).absolutePath());

        //Create the file, removing the old log file if it exists
        QFile file(LOG_FILE_PATH);
        file.open(QIODevice::WriteOnly | QIODevice::Text);

        QTextStream stream(&file);
        stream << "Sirve app was launched, beginning a new log file." << endl;
        qInstallMessageHandler(customMessageHandler);
    }

	QApplication a(argc, argv);
	SirveApp w;
	w.show();
	
	a.exec();

	return 0;
}
