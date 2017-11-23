*****************   *****************   *****************   *****************
# Connection Statistics Analyzer
  (aka 'ConnStat')
  The connstat library allows you to check how good the connectivity to 
  a specific URL, collects and analyze statistics such as median time of connection.

  This project contains 3 parts:
	1) libconnstat library  // The library itself
	2) connstat_tests       // Runs UT for the lib
	3) connstat_runner      // Executable with cmd input from user to communicate with the lib

  It is using the libCURL 'easy' interface (see  https://curl.haxx.se/libcurl/c/)
  It is part of an excersize test for SamKnows (https://www.samknows.com)
     See More details: https://github.com/SamKnows/tests-and-metrics-test

*****************   *****************   *****************   *****************
## Getting Started
### Compiling just the library
You can compile the library so it will create a DLL file by running libconnstat/makefile.
Your DLL will be placed under the libconnstat/bin folder.

### Running the tests
If you want to run the tests you can just run the connstat_tests/makefile.
It will compile the lib as well as the tests and create an executable under connstat_tests/lib.
run it by using: ./bin/connstat_tests.exe

### Running with command line and arguments
If you want to use the executable runner (so you can send args to the lib) run the connstat_runner/makefile.
It will compile the lib as well as the tests and create an executable under connstat_tests/lib.
run it by using: ./bin/connstat_tests.exe
./bin/connstat_runner.exe
You can run it like this for example:
./bin/connstat_runner.exe -n 4 -H "Keep-Alive: 300" -H "Connection: keep-alive"

*****************   *****************   *****************   *****************
### Installing

*****************   *****************   *****************   *****************
## Future work and side notes to SamKnow reviewer:
Given more time & effort, I would do the following:
 - Create a wrapper to the usage of libcurl
    * Let's call it connLibWrapper
	* My code/lib won't be dependent on CURL. The wrapper will be the only one to include and call CURL 
	* This will allow easy maintainance in case API is changed (whether libcurl API is changed or even choosing different lib in the future)
 - Create Logger infrastructure with verbosity level 
	* Currently some logs are printf, some goes to stderr
	* Logger will contain module name, "\n" (so no need to type "\n" at every log)
	* Logger will be placed in a dedicated file
	* It will re-use the logs coming out of libcurl
	* Allow configuring level from console as a program argument
 - Better Error handling
    * Call connection_stats_close @ every error
	* Map which errors are assert and which are soft errors (maybe can be covered)
	* Insead of print RC as a number I would create a converter MACRO to get it as a string
	* Add more options to the RC type
 - UI/UX - Console
    * Create an help menu to be displayed when user uses the lib incorrectly
	* Add more arguments related to logs level
 - Add more statistics
    * Get more info from the CURL library - requires better understanding of HTTP timings analyses 
 - Check curl_version_info() at init run time
 - Hold an instance (handle) of the library so all operations will be performed on it
 - Combine 2 makefiles (connstat_tests & connstat_runner) into 1 makefile with args (99% identical)
 - makefiles should clean folders as well, not just the content.
 - Add debug capabilities
 - Add versioning
 - Think of security issues- mainly working with strings (url, http header, program output)
 
*****************   *****************   *****************   *****************
## Useful examples this project is using
Useful examples from official CURL website:
	https://curl.haxx.se/libcurl/c/debug.html
	https://curl.haxx.se/libcurl/c/httpcustomheader.html
	https://curl.haxx.se/libcurl/c/sepheaders.html

*****************   *****************   *****************   *****************

## Authors
Omri Ravid - Initial and maintainer

## License
Open Source

## Acknowledgments
Thanks to SamKnows who inspired me of doing this project ;-)


