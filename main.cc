#include "passmanapp.h"

int main(int argc, char *argv[])
{
	QCoreApplication::setApplicationName("PassMan");
	QCoreApplication::setApplicationVersion("0.1.1");
	QCoreApplication::setOrganizationName("LtKStudios");
	QCoreApplication::setOrganizationDomain("ltkstudios.net");

	PassManApp app(argc, argv);

	app.init();

	return app.exec();
}
