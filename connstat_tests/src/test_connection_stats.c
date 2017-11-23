/*
 * test_connection_stats.c
 *
 *  Created on: 21 Nov 2017
 *      Author: Omri Ravid
 *
 * This program should test the connection_stats library.
 *
 * connection_stats library allows you to check how good the connectivity to 
 * a specific URL.
 * It is using the libCURL 'easy' interface (see  https://curl.haxx.se/libcurl/c/)
 * It is part of an excersize test for SamKnows (https://www.samknows.com)
 *    See More details: https://github.com/SamKnows/tests-and-metrics-test

 */

#include <stdio.h>
#include <string.h>
#include <../libconnstat/inc/connection_stats.h>

/*
Future Tests:

Black Box:
- validate num_of_http_req range
- validate URL (not too long, something else?)
- Validate http header (from user)
- no memory leaks

UT:
- Trigger before init should fail
- simulate as if libcurl returned error (in some places) - program shall not crash and report it
- simulate as if libcurl edge-case values when using curl_easy_getinfo
- fail to open files
- Validate getMedian for odd/even/empty arrays
*/

static int test_num_of_http_req();
static int test_invalid_url();
static int test_invalid_http_header();

/**
* @func:  main
* @desc:  Main entry function - Runs all tests in this file
* @return 0 if success, 1 otherwise
*/
int main() {
	int rc;
	
	printf("#####  Start running tests... \n");
	
	rc = test_invalid_url();
	if (rc != 0) {
		printf("test_invalid_url() failed \n");
		return 1;
	}
	
	rc = test_num_of_http_req();
	if (rc != 0) {
		printf("test_num_of_http_req() failed \n");
		return 1;
	}
	
	rc = test_invalid_http_header();
	if (rc != 0) {
		printf("test_invalid_http_header() failed \n");
		return 1;
	}	
	
	printf("\n\n##### All tests pass! \n");
	return 0;
}

/**
* @func:  test_num_of_http_req
* @desc:  Validate that number of HTTP requests is supported (array limitation)
* @return 0 if test pass, 1 otherwise
*/
static int test_num_of_http_req() {
	RC rc;
	
	/* Init connection_stats library */
	rc = connection_stats_init();
	if (rc != RC_OK) {
		printf("test_num_of_http_req fail: connection_stats_init() returned rc=%d \n", rc);
		connection_stats_close();
		return 1;
	}
	
	HttpReqData http_req_data;
	memcpy(http_req_data.url, DEFAULT_URL, DEFAULT_URL_SIZE);
	
	/* Expect failure when num of requests is 0 */
	http_req_data.num_of_http_req = 0;
	rc = connection_stats_trigger(&http_req_data);
	if (rc != RC_INVALID_NUM_OF_HTTP_REQ) {
		printf("test_num_of_http_req fail: connection_stats_trigger() should fail for 0. rc=%d \n", rc);
		connection_stats_close();
		return 1;
	}

	/* Expect failure when num of requests is larger than max supported */
	http_req_data.num_of_http_req = MAX_NUM_OF_SUPPORTED_CURL_OPER + 1;
	rc = connection_stats_trigger(&http_req_data);
	if (rc != RC_INVALID_NUM_OF_HTTP_REQ) {
		printf("test_num_of_http_req fail: connection_stats_trigger() should fail for MAX+1. rc=%d \n", rc);
		connection_stats_close();
		return 1;
	}	
	
	/* Expect SUCCESS when num of requests is in correct range */
	http_req_data.num_of_http_req = MAX_NUM_OF_SUPPORTED_CURL_OPER;
	rc = connection_stats_trigger(&http_req_data);
	if (rc != RC_OK) {
		printf("test_num_of_http_req fail: connection_stats_trigger() should succeedd for MAX. rc=%d \n", rc);
		connection_stats_close();
		return 1;
	}	

	printf("test_num_of_http_req  ..........  test PASS\n");

	/* Close library, here and in every failure above */
	connection_stats_close();
	return 0;	
}

/**
* @func:  test_num_of_http_req
* @desc:  Validate that given URL makes sense
* @return 0 if test pass, 1 otherwise
*/
static int test_invalid_url() {
	RC rc;
		
	rc = connection_stats_init();
	if (rc != RC_OK) {
		printf("test_num_of_http_req fail: connection_stats_init() returned rc=%d \n", rc);
		return 1;
	}
	HttpReqData http_req_data;
	http_req_data.num_of_http_req = 1;

	/* Intentioally set URL to "" */
	//http_req_data.url = "";
	
	/* Expect failure */
	rc = connection_stats_trigger(&http_req_data);
	if (rc != RC_INVALID_URL) {
		printf("test_invalid_url fail: connection_stats_trigger() should fail for NULL. rc=%d \n", rc);
		connection_stats_close();
		return 1;
	}
	
	printf("test_invalid_url  ..........  test PASS\n");

	/* Close library, here and in every failure above */
	connection_stats_close();
	return 0;
}

/**
* @func:  test_invalid_http_header
* @desc:  Validate that given HTTP HEADER makes sense
* @return 0 if test pass, 1 otherwise
*/
static int test_invalid_http_header() {
	RC rc;
		rc = connection_stats_init();
	if (rc != RC_OK) {
		printf("test_invalid_http_header fail: connection_stats_init() returned rc=%d \n", rc);
		return 1;
	}
	
	/* Expect failure when http_header is NULL */
	rc = connection_stats_add_http_hdr(NULL);
	if (rc != RC_INVALID_HTTP_HEADER) {
		printf("test_invalid_http_header fail: Expected failure for NULL (rc=%d)\n", rc);
		return 1;
	}
	
	/* Expect failure when http_header doesn't include ':' */
	char* invalid_http_header = "header_only";
	rc = connection_stats_add_http_hdr(invalid_http_header);
	if (rc != RC_INVALID_HTTP_HEADER) {
		printf("test_invalid_http_header fail: Expected failure for header only (rc=%d)\n", rc);
		return 1;
	}
	
	/* Expect success for valid http_header */
	char* valid_http_header = "header: value";
	rc = connection_stats_add_http_hdr(valid_http_header);
	if (rc != RC_OK) {
		printf("test_invalid_http_header fail: Expected success for valid http_header (rc=%d)\n", rc);
		return 1;
	}
	
	printf("test_invalid_http_header  ..........  test PASS\n");

	/* Close library, here and in every failure above */
	connection_stats_close();
	
	return 0;
}
