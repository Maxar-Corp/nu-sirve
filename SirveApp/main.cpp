//#define _CRTDBG_MAP_ALLOC
#include "SirveApp.h"
#include <QtWidgets/QApplication>

#include <iostream>
#include "logging.h"
#include "annotation_list_dialog.h"

int main(int argc, char *argv[])
{
	
	INFO << "Main: Application started";

	QApplication a(argc, argv);
	SirveApp w;
	w.show();
	
	a.exec();

	return 0;
}
