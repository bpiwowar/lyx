/**
 * \file os.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Ruurd A. Reitsma
 * \author Enrico Forestieri
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "debug.h"
#include "filetools.h"
#include "qstring_helpers.h"

#include <QDir>

#if defined(__CYGWIN__)
#include "support/os_cygwin.cpp"
#elif defined(_WIN32)
#include "support/os_win32.cpp"
#else
#include "support/os_unix.cpp"
#endif

// Static assert to break compilation on platforms where
// int/unsigned int is not 4 bytes. Added to make sure that
// e.g., the author hash is always 32-bit.
template<bool Condition> struct static_assert_helper;
template <> struct static_assert_helper<true> {};
enum { 
	dummy = sizeof(static_assert_helper<sizeof(int) == 4>)
};

namespace lyx {
namespace support {
namespace os {

static string const python2(string const & binary, bool verbose = false)
{
	if (verbose)
		lyxerr << "Examining " << binary << "\n";

	// Check whether this is a python 2 binary.
	cmd_ret const out = runCommand(binary + " -V 2>&1");
	if (out.first < 0 || !prefixIs(out.second, "Python 2"))
		return string();

	if (verbose)
		lyxerr << "Found " << out.second << "\n";
	return binary;
}


int timeout_min()
{
	return 3;
}


string const python(bool reset)
{
	// Check whether the first python in PATH is the right one.
	static string command = python2("python -tt");
	if (reset) {
		command = python2("python -tt");
	}

	if (command.empty()) {
		// It was not, so check whether we can find it elsewhere in
		// PATH, maybe with some suffix appended.
		vector<string> const path = getEnvPath("PATH");
		vector<string>::const_iterator it = path.begin();
		vector<string>::const_iterator const end = path.end();
		lyxerr << "Looking for python v2.x ...\n";
		for (; it != end; ++it) {
			QString const dir = toqstr(*it);
			string const localdir = dir.toLocal8Bit().constData();
			QDir qdir(dir);
			qdir.setFilter(QDir::Files | QDir::Executable);
			QStringList list = qdir.entryList(QStringList("python*"));
			for (int i = 0; i < list.size() && command.empty(); ++i) {
				string const binary = addName(localdir,
					list.at(i).toLocal8Bit().constData());
				command = python2(binary, true);
			}
		}

		// Default to "python" if no usable binary was found.
		if (command.empty()) {
			lyxerr << "Warning: No python v2.x binary found.\n";
			command = "python";
		}

		// Add the -tt switch so that mixed tab/whitespace
		// indentation is an error
		command += " -tt";
	}
	return command;
}

}
}
}
