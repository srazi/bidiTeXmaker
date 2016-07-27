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
		return "11Dec2013, 12:43:00";
	}

	inline const char* globalRevision()
	{
		return "798634712e2a";
	}
}

#endif // VERSION_H
