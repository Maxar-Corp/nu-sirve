//#define _CRTDBG_MAP_ALLOC
#include "QtGuiApplication1.h"
#include <QtWidgets/QApplication>

#include <iostream>
#include "logging.h"
#include "annotations.h"

int main(int argc, char *argv[])
{
	
	INFO << "Main: Application started";

	QApplication a(argc, argv);
	QtGuiApplication1 w;
	w.show();
	
	a.exec();

	return 0;
}
