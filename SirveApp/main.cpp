//#define _CRTDBG_MAP_ALLOC
#include "SirveApp.h"
#include <QtWidgets/QApplication>

#include <iostream>
#include "logging.h"
#include "annotation_list_dialog.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>

void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    QFile file("sirveapp_debug_log.txt");
    file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);

    // Write the message to the file
    QTextStream stream(&file);
    stream << msg << endl;

    // Also output the message to the console
    // QTextStream(stdout) << msg << endl;
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(customMessageHandler);

	INFO << "Main: Application started";

	QApplication a(argc, argv);
	SirveApp w;
	w.show();
	
	a.exec();

	return 0;
}
