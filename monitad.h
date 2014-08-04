#ifndef __monitad_h__
#define __monitad_h__

#include "common.h"

int 	m_argc;
char** 	m_argv;

class monitad
{
	private:

	public:
		monitad();
		~monitad();

		int system(const char* cmdstring);
};

#endif
