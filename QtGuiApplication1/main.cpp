//#define _CRTDBG_MAP_ALLOC
#include "QtGuiApplication1.h"
#include <QtWidgets/QApplication>

//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>
//#ifdef _DEBUG
//#define _CRTDBG_MAP_ALLOC
//#define _INC_MALLOC
//#endif


int main(int argc, char *argv[])
{
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	QApplication a(argc, argv);
	QtGuiApplication1 w;
	w.show();
	
	a.exec();

	//_CrtDumpMemoryLeaks();
	
	return 0;
}
