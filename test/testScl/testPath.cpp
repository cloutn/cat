#include "testPath.h"

#include "scl/path.h"

#include <stdio.h>

using namespace scl;

void testPath()
{
	char home[256] = { 0 };
	get_home_path(home, sizeof(home));
	printf("test path : home\t= [%s]\n", home);

	char app[256] = { 0 };
	get_application_path(app, sizeof(app));
	printf("test path : app\t= [%s]\n", app);
}
