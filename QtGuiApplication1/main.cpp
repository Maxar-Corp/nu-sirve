//#define _CRTDBG_MAP_ALLOC
#include "QtGuiApplication1.h"
#include <QtWidgets/QApplication>

#include <iostream>
#include "logging.h"

//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>
//#ifdef _DEBUG
//#define _CRTDBG_MAP_ALLOC
//#define _INC_MALLOC
//#endif

/*
#include <boost/move/utility_core.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/severity_feature.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

//namespace logging = boost::log;
//namespace src = boost::log::sources;
//namespace keywords = boost::log::keywords;

//BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(my_logger, src::logger_mt)


void init()
{
	//logging::register_simple_formatter_factory<logging::trivial::severity_level, char>("Severity");

	logging::add_file_log
	(
		keywords::file_name = "sample.log",
		keywords::format = "[%TimeStamp%] [%ThreadID%] [%Severity%] %Message%"
	);

	//logging::core::get()->set_filter
	//(
	//	logging::trivial::severity >= logging::trivial::debug
	//);

	logging::add_common_attributes();
}

*/

int main(int argc, char *argv[])
{
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	//init();

	//src::logger_mt& lg = my_logger::get();

	//BOOST_LOG_SEV(lg, trace) << "A trace severity message";
	//BOOST_LOG_SEV(lg, debug) << "A debug severity message";
	//BOOST_LOG_SEV(lg, info) << "An informational severity message";
	//BOOST_LOG_SEV(lg, warning) << "A warning severity message";
	//BOOST_LOG_SEV(lg, error) << "An error severity message";
	//BOOST_LOG_SEV(lg, fatal) << "A fatal severity message";
	
	INFO << "Main: Application started";

	QApplication a(argc, argv);
	QtGuiApplication1 w;
	w.show();
	
	a.exec();

	//_CrtDumpMemoryLeaks();

	
	
	return 0;
}
