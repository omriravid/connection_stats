/*
 * Main.c
 *
 *  Created on: 23 Nov 2017
 *      Author: Omri Ravid
 * 
 * This program allows users to execute and use the connection_stats library.
 * connection_stats library allows you to check how good the connectivity to 
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
#include <unistd.h> /* Parsing using getopt */
#include <../libconnstat/inc/connection_stats.h>

/******************
**    Methods    **
******************/
/**
* @func:  parse_args
* @desc:  Parse user (console) inputs to retrieve data such as
*		  Number of HTTP requests, URL, HTTP additional headers, etc..
* @param  argc	according to program arguments as received by the user 
* @param  argv	according to program arguments as received by the user 
* @param  p_http_req_data    Pointer to HttpReqData to be filled by the parser 
* @return 0 if success, 1 otherwise
*/
static int parse_args(int argc, char *argv[], HttpReqData *p_http_req_data) {
	int opt;

	/* Set default values before parsing */
	p_http_req_data->num_of_http_req = DEFAULT_NUM_OF_HTTP_REQ;
	memcpy(p_http_req_data->url, DEFAULT_URL, DEFAULT_URL_SIZE); 
	
	while ((opt = getopt (argc, argv, "n:u:H:")) != -1)
	{
		switch (opt)
		{
			case 'n':
				p_http_req_data->num_of_http_req = atoi(optarg);
				break;
			
			case 'u':
				if (strlen(optarg) > URL_MAX_LEN) {
					printf("Requested URL is too long (%d>%d) \n", 
							strlen(optarg), URL_MAX_LEN);
					return RC_PARSING_ERROR;
				}
				memcpy(p_http_req_data->url, optarg, strlen(optarg));
				break;
				
			case 'H':
				connection_stats_add_http_hdr(optarg);
				break;
				
			case '?':
				return RC_PARSING_ERROR;
		}
	}
	return RC_OK;
}

/**
* @func:  main
* @desc:  The main function of the program.
*         It parses user input, then call the connection_stats library
*         with the following sequence: Init->Trigger->Analyze->Close
* @param  argc	according to program arguments as received by the user 
* @param  argv	according to program arguments as received by the user 
* @return 0 if success, 1 otherwise
*/
int main(int argc, char *argv[]){	
	HttpReqData http_req_data;	
	int rc;
		
	/* Initialize the library (include init for the lib CURL) */
	rc = connection_stats_init();
	if (rc != RC_OK) {
		printf ("connection_stats_init() failed: (rc=%d) \n", rc);
		connection_stats_close();
		return 1;
	}
	
	/* Parse user's args and build data to later forward to the library */
	rc = parse_args(argc, argv, &http_req_data);
	if (rc != RC_OK) {
		printf ("parse_args() failed: (rc=%d) \n", rc);
		connection_stats_close();
		return 1;
	}
	
	/* Trigger the library to collect and analyze data */
	rc = connection_stats_trigger(&http_req_data);
	if (rc != RC_OK) {
		printf ("connection_stats_trigger() failed: (rc=%d) \n", rc);
		connection_stats_close();
		return 1;
	}
	
	/* Close the library */
	rc = connection_stats_close();
	if (rc != RC_OK) {
		printf ("connection_stats_close() failed: (rc=%d) \n", rc);
		return 1;
	}

	char statistics_result[MAX_SIZE_OF_PROG_OUTPUT];
	size_t strLen;
	rc = connection_stats_get_statistics(statistics_result, &strLen);
	if (rc != RC_OK) {
		printf ("connection_stats_get_statistics() failed: (rc=%d) \n", rc);
		return 1;
	}
	printf("runner: %s (strLen=%d)\n", statistics_result, strLen);
	
	return 0;
}
