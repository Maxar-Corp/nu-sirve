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
    stream << msg << "\r\n";

    // Also output the message to the console
    if (type == QtInfoMsg || type == QtDebugMsg) {
        // If it's not an info or debug message, we can also print it to stderr
        QTextStream(stdout) << msg << "\r\n";
    } else {
        QTextStream(stderr) << msg << "\r\n";
    }
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
        stream << "Sirve app was launched, beginning a new log file." << "\r\n";
        qInstallMessageHandler(customMessageHandler);
    }

    QApplication app(argc, argv);
    QApplication::setApplicationName("SIRVE");

    QFile file(":/stylesheets/Modified_Ubuntu.qss");
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        app.setStyleSheet(file.readAll());
        file.close();
    }

    SirveApp sirve;

    QPointer screen = QApplication::primaryScreen();

    QSize resolution = screen->size();

    if (resolution.width() <= 1920)
    {
        sirve.move(10,10);
    }
    else
    {
        sirve.move((resolution.width() - sirve.size().width())/2,(resolution.height() - sirve.size().height())/2);
    }

    sirve.show();

    qDebug() << "Dimensions of Sirve: Width=" << sirve.size().width() << " Height=" << sirve.size().height() << "\r\n";

    return app.exec();
}
