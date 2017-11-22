/*
 * connection_stats.h
 *
 *  Created on: 22 Nov 2017
 *      Author: Omri Ravid
 * 
 * This is the H file (API) exposed by the libconnstat library.
 * libconnstat library allows you to check how good the connectivity to 
 * a specific URL.
 * It is using the libCURL 'easy' interface (see  https://curl.haxx.se/libcurl/c/)
 * It is part of an excersize test for SamKnows (https://www.samknows.com)
 *    See More details: https://github.com/SamKnows/tests-and-metrics-test
 */

/******************
**   Includes    **
******************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "../inc/connection_stats.h"


/******************
**   Defines     **
******************/


/******************
**  Structures   **
******************/


/******************
**  Global Vars  **
******************/
static CURL *g_curl;


/*************************
** Methods Declerations **
*************************/

static RC is_valid_http_data_req(HttpReqData *p_http_req_data);

/******************
**    Methods    **
******************/

/**
* @desc   Initialize the library (including initialization of libCURL)
* @return Return Code (taken from RC enum)
*/
RC connection_stats_init() {
	CURLcode res;
	
	/* Initialize libCURL easy interface */
	
	res = curl_global_init(CURL_GLOBAL_DEFAULT);
	if (res != CURLE_OK) {
		printf("connection_stats_init() fail with curl_global_init(): %s\n", 
				curl_easy_strerror(res));
		return RC_ERROR_IN_CURL;
	}

	g_curl = curl_easy_init();
	if (g_curl == NULL) {
		printf("connection_stats_init() fail with curl_easy_init() \n");
		return RC_ERROR_IN_CURL;
	}
	
	return RC_OK;
}

/**
* @desc   Close the library gracefully (including closing files and libCURL insstance) 
* @return Return Code (taken from RC enum)
*/
RC connection_stats_close() {

	/* Cleanup CURL before leaving the program*/
	curl_easy_cleanup(g_curl);
	curl_global_cleanup();
	
	// TODO: does any of the above return RC? use it..
	return RC_OK;
}

RC connection_stats_trigger(HttpReqData *p_http_req_data) {
	CURLcode res;
	CurlInfo curl_info_arr[MAX_NUM_OF_SUPPORTED_CURL_OPER];
	
	RC rc = is_valid_http_data_req(p_http_req_data);
	if (rc != RC_OK) {
		return rc;
	}
	
	printf("connection_stats_trigger() called [num_of_http_req=%d, url=%s]\n",
			p_http_req_data->num_of_http_req, p_http_req_data->url);
	
	/* Set all easy curl options */
	
	/* Set lib CURL option for URL */
	res = curl_easy_setopt(g_curl, CURLOPT_URL, p_http_req_data->url);
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_setopt() failed CURLOPT_URL: %s\n", 
				curl_easy_strerror(res));
		return RC_ERROR_IN_CURL;
	}
	
	/* Set lib CURL option for following redirection */
    res = curl_easy_setopt(g_curl, CURLOPT_FOLLOWLOCATION, 1L);
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_setopt() failed CURLOPT_FOLLOWLOCATION: %s\n", 
				curl_easy_strerror(res));
		return RC_ERROR_IN_CURL;
	}

	/* Perform the operation (using curl) multiple times (as requested by user) */
	for (int i=0; i<p_http_req_data->num_of_http_req; i++) {
		/* Perform the curl request */
		res = curl_easy_perform(g_curl);
		if(res != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() failed: %s\n",	
					curl_easy_strerror(res));
			return RC_ERROR_IN_CURL;
		}
		
		/* Collect statistics */
		// TODO: Collect the statistics and calc median
	} // End of FOR loop

	return RC_OK;
}
