//#define _CRTDBG_MAP_ALLOC
#include "SirveApp.h"
#include <QtWidgets/QApplication>

#include <QDebug>
#include <QFile>
#include <QTextStream>

void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    QString LOG_FILE_PATH = "log/debug_log_file.txt";
    QFile file(LOG_FILE_PATH);
    file.open(QIODevice::Append | QIODevice::Text);

    // Write the message to the file
    QTextStream stream(&file);
    //stream << msg << endl;

    // Also output the message to the console
    // QTextStream(stdout) << msg << endl;
}

int main(int argc, char *argv[])
{
	// Check the command line argument count before checking if the first true argument is a debug flag
    if (argc > 1 && std::strcmp(argv[1],"--debug") == 0) {
        QString LOG_FILE_PATH = "log/debug_log_file.txt";

        //Create the intermediate directories if they don't exist
        QDir().mkpath(QFileInfo(LOG_FILE_PATH).absolutePath());

        //Create the file, removing the old log file if it exists
        QFile file(LOG_FILE_PATH);
        file.open(QIODevice::WriteOnly | QIODevice::Text);

        QTextStream stream(&file);
        stream << "Sirve app was launched, beginning a new log file.";
        qInstallMessageHandler(customMessageHandler);
    }

	QApplication a(argc, argv);

    QFile file(":/stylesheets/Modified_Ubuntu.qss");
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        a.setStyleSheet(file.readAll());
        file.close();
    }

    SirveApp app;
    app.show();

    qDebug() << "Dimensions of Sirve: Width=" << app.size().width() << " Height=" << app.size().height();

    return a.exec();
}
