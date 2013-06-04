#ifndef VERSION_H
#define VERSION_H

namespace Version
{
	const int MAJOR = 2;
	const int MINOR = 1;
	const int LOCAL_REVISION = 33;
	const int BUILD = 5420;

	inline const char* versionString()
	{
		return "2.1.32.5420";
	}

	inline const char* versionDate()
	{
		return "03Jun2013, 14:17:04";
	}

	inline const char* globalRevision()
	{
		return "f9428b252c2a";
	}
}

#endif // VERSION_H
