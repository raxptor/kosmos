#ifndef __KOSMOS_LOG_H__
#define __KOSMOS_LOG_H__

namespace kosmos
{
	void log(const char *cstr);    // needed by macros.
	void error(const char *cstr);

	void syslog(const char *cstr); // output to console
}

#if KOSMOS_ENABLE_LOG

	#include <sstream>
	
	#define KOSMOS_INFO(stmt) {                          \
		std::ostringstream __DPRINT_LINE;                  \
		__DPRINT_LINE << __FILE__ << " (" << __LINE__ << "): " << stmt; \
		kosmos::log(__DPRINT_LINE.str().c_str()); \
	}

	#define KOSMOS_ERROR(stmt) {                         \
		std::ostringstream __DPRINT_LINE;                  \
		__DPRINT_LINE << __FILE__ << " (" << __LINE__ << "): " << stmt; \
		kosmos::error(__DPRINT_LINE.str().c_str()); \
	}

#else
	#define KOSMOS_INFO(stmt) { }
	#define KOSMOS_ERROR(stmt) { }
#endif

#define KOSMOS_WARNING(x) KOSMOS_INFO("warn: " << x)
#define KOSMOS_DEBUG(x) KOSMOS_INFO("debug:" << x)

#endif
