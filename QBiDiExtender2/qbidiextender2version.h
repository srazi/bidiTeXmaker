#ifndef VERSION_H
#define VERSION_H

namespace Version
{
	const int MAJOR = 2;
	const int MINOR = 0;
	const int LOCAL_REVISION = 32;
	const int BUILD = 5404;

	inline const char* versionString()
	{
		return "2.0.32.5404";
	}

	inline const char* versionDate()
	{
		return "28May2013, 22:02:08";
	}

	inline const char* globalRevision()
	{
		return "d8206f0fef47";
	}
}

#endif // VERSION_H
