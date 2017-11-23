/*
 * connection_stats.h
 *
 *  Created on: 21 Nov 2017
 *      Author: Omri Ravid
 * 
 * This is the H file (API) exposed by the libconnstat library.
 * libconnstat library allows you to check how good the connectivity to 
 * a specific URL.
 * It is using the libCURL 'easy' interface (see  https://curl.haxx.se/libcurl/c/)
 * It is part of an excersize test for SamKnows (https://www.samknows.com)
 *    See More details: https://github.com/SamKnows/tests-and-metrics-test
 */
 
#ifndef CONNECTIONSTATS_H_
#define CONNECTIONSTATS_H_

/******************
**   Includes    **
******************/

/******************
**    Defines    **
******************/
#define MAX_NUM_OF_SUPPORTED_CURL_OPER  16 /* This can be easily extendded
                                             and in case it is most likely to 
											 be extendded we can use vector */
#define DEFAULT_NUM_OF_HTTP_REQ         1
#define DEFAULT_URL                     "http://www.google.com/"
#define DEFAULT_URL_SIZE                strlen(DEFAULT_URL)
#define MAX_SIZE_OF_PROG_OUTPUT         64
#define URL_MAX_LEN                     64
#define URL_MIN_LEN                     5
#define HTTP_HEADER_MAX_LEN             64
#define HTTP_HEADER_MIN_LEN             2



/******************
**     Enums     **
******************/
/**
* Return Code possible values
*/
typedef enum
{
	RC_OK 		= 0,
	RC_ERROR,
	RC_INVALID_NUM_OF_HTTP_REQ,
	RC_INVALID_URL,
	RC_INVALID_HTTP_HEADER,
	RC_ERROR_IN_CURL
} RC;


/******************
**  Structures   **
******************/
/**
* HTTP data - the connection_stats library will operate accordingly
*/
 typedef struct {
  int 		num_of_http_req;  /* Number of HTTP requests to make */
  char 		url[URL_MAX_LEN]; /* Target URL */
} HttpReqData;


/******************
**    Methods    **
******************/

/**
* @desc   Initialize the library (including initialization of libCURL)
* @return Return Code (taken from RC enum)
*/
RC connection_stats_init();

/**
* @desc   Add an extra HTTP header to the request
* @param  http_header	HTTP header to be added to CURL request 
*                       (In format: "Header-name: Header-value")
* @return Return Code (taken from RC enum)
*/
RC connection_stats_add_http_hdr(char* http_header);

/**
* @desc   Trigger for the library to execute HTTP request
*         According to the previously provided arguments.
* @param  http_req_data	Data as received by the user 
* @return Return Code (taken from RC enum)
*/
RC connection_stats_trigger(HttpReqData* http_req_data);

/**
* @desc   Collect all required info about the connection and generate statistics 
* @return Return Code (taken from RC enum)
*/
RC connection_stats_analyze();

/**
* @desc   Close the library gracefully (including closing files and libCURL insstance) 
* @return Return Code (taken from RC enum)
*/
RC connection_stats_close();

#endif /* CONNECTIONSTATS_H_ */
