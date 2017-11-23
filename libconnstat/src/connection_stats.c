/*
 * connection_stats.c
 *
 *  Created on: 22 Nov 2017
 *      Author: Omri Ravid
 * 
 * This is the implmentation file of the libconnstat library.
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
#include <dirent.h>   // opendir()
#include <sys/stat.h> // mkdir
#include <curl/curl.h>
#include "../inc/connection_stats.h"


/******************
**   Defines     **
******************/
#define MAX_SIZE_OF_IP_ADD      46 // IPv4=15, IPv6=45 (+1 for null terminating char) // TODO: verify the +1
#define TRACE_ENA               1  // TODO: should I deliver where it is defined or not?
#define USE_BODY_HEADER_FILES   1  // TODO: rethink if required


/******************
**  Structures   **
******************/
/* Info attributes to be retrieved from the CURL lib */
typedef struct  {
	double name_lookup_time;
	double connect_time;
	double start_transfer_time;
	double total_time;
} CurlInfo;

#ifdef TRACE_ENA
struct data {
  char trace_ascii; /* 1 or 0 */ 
};
#endif

struct url_data {
  char *ptr;
  size_t len;
};


/******************
**  Global Vars  **
******************/
/* Handle for curl operations */ 
static CURL *g_curl;

/* List of all http headers to be added to the CURL request */
static struct curl_slist *g_http_headers_curl_list = NULL;

/* Prog/Lib output string */
static char g_prog_output[MAX_SIZE_OF_PROG_OUTPUT];

#ifdef USE_BODY_HEADER_FILES
FILE *g_header_file;
FILE *g_body_file;
#endif

#ifdef TRACE_ENA
FILE *g_trace_file;
#endif

/*************************
** Methods Declerations **
*************************/
#ifdef TRACE_ENA
static void dump(const char *text, FILE *stream, unsigned char *ptr, 
				size_t size, char nohex);
static int trace_func(CURL *handle, curl_infotype type, char *data, 
					  size_t size, void *userp);
#endif // TRACE_ENA
static void init_string(struct url_data *url_data);
#ifdef WRITEFUNC_USED
static size_t write_func(void *ptr, size_t size, size_t nmemb, struct url_data *url_data);
#endif // WRITEFUNC_USED
static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream);
static int double_comp(const void* elem1, const void* elem2);
static double get_median(double arr[], int arr_size);
static RC open_trace_files();
static RC is_valid_http_data_req(HttpReqData *p_http_req_data);
static RC is_valid_http_header(char* http_header);

/******************
**    Methods    **
******************/
/**
* @desc   Collect all required info about the connection  
* @return Return Code (taken from RC enum)
*/
static RC connection_stats_collect(CurlInfo* curl_info) {	
	CURLcode res;
	
	// Get Name Lookup Time
	res = curl_easy_getinfo(g_curl, CURLINFO_NAMELOOKUP_TIME, 
							&curl_info->name_lookup_time);
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_getinfo() failed CURLINFO_NAMELOOKUP_TIME: %s\n",	
				curl_easy_strerror(res));
		return RC_ERROR_IN_CURL;
	}
	
	// Get Connet Time
	res = curl_easy_getinfo(g_curl, CURLINFO_CONNECT_TIME, 
							&curl_info->connect_time);
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_getinfo() failed CURLINFO_CONNECT_TIME: %s\n",	
				curl_easy_strerror(res));
		return RC_ERROR_IN_CURL;
	}
	
	// Get Start Transfer Time
	res = curl_easy_getinfo(g_curl, CURLINFO_STARTTRANSFER_TIME, 
							&curl_info->start_transfer_time);
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_getinfo() failed CURLINFO_STARTTRANSFER_TIME: %s\n",	
				curl_easy_strerror(res));
		return RC_ERROR_IN_CURL;
	}
	
	// Get Total Time
	res = curl_easy_getinfo(g_curl, CURLINFO_TOTAL_TIME, 
							&curl_info->total_time);
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_getinfo() failed CURLINFO_TOTAL_TIME: %s\n",	
				curl_easy_strerror(res));
		return RC_ERROR_IN_CURL;
	}
	
	return RC_OK;
}

/**
* @desc   Collect all required info about the connection and generate statistics 
* @return Return Code (taken from RC enum)
*/
RC connection_stats_analyze(CurlInfo* curl_info_arr, int arr_size) {
	CURLcode res;
	int i=0;
	/* Note: As always, we have a tradeoff here, between time and complexity.
	         We can create an array per each of the statistics
			    O(n*m) where 
				    n is arr_size (number of HTTP requests) 
					m is the number of different statistics (4 in our case) 
			 Or we can save space but then we will run m*o(n) 
			    and each time we will take different statistics.
			 I chose the first approach as it is much more readable */
	double name_lookup_time_arr[arr_size];
	double connect_time_arr[arr_size];
	double start_transfer_time_arr[arr_size];
	double total_time_arr[arr_size];
	
	// Copy to temporal arrays
	for (i=0; i<arr_size; i++) {
		name_lookup_time_arr[i]    = curl_info_arr[i].name_lookup_time;
		connect_time_arr[i]        = curl_info_arr[i].connect_time;
		start_transfer_time_arr[i] = curl_info_arr[i].start_transfer_time;
		total_time_arr[i]          = curl_info_arr[i].total_time;
		
		// TODO: Log this..
		printf("   # %d:  ", i);
		printf("name_lookup_time=%.6f ;; ",   curl_info_arr[i].name_lookup_time);
		printf("connect_time=%.6f ;; ",       curl_info_arr[i].connect_time);
		printf("start_transfer_time=%.6f ;; ",curl_info_arr[i].start_transfer_time);
		printf("total_time=%.6f ;; ",         curl_info_arr[i].total_time);
		printf("\n");	
	}
	
	// Get Median per each array
	double name_lookup_time_median    = get_median(name_lookup_time_arr, arr_size);
	double connect_time_median        = get_median(connect_time_arr, arr_size);
	double start_transfer_time_median = get_median(start_transfer_time_arr, arr_size);
	double total_time_median          = get_median(total_time_arr, arr_size);
	
	// Get IP Adress
	char *ip;
	res = curl_easy_getinfo(g_curl, CURLINFO_PRIMARY_IP, &ip) && ip;
	if ((res != CURLE_OK) || (ip==NULL)){
		fprintf(stderr, "curl_easy_getinfo() failed CURLINFO_PRIMARY_IP: %s\n",	
		curl_easy_strerror(res));
		return RC_ERROR_IN_CURL;
	}
	// printf("IP: %s \n", ip);
	/* Note that we get a pointer to a memory area that will be re-used
	        at next request, so we need to copy the string if we want to 
	        keep the information. */
	//memcpy(curl_info->ip, ip, MAX_SIZE_OF_IP_ADD);
	
	long response_code;
	res = curl_easy_getinfo(g_curl, CURLINFO_RESPONSE_CODE, &response_code);
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_getinfo() failed CURLINFO_RESPONSE_CODE: %s\n",	
				curl_easy_strerror(res));
		return RC_ERROR_IN_CURL;
	}
	
	/* Print program's output in the following format:
	   SKTEST;<IP address of HTTP server>;<HTTP response code>;
	          <median of CURLINFO_NAMELOOKUP_TIME>;
	  		  <median of CURLINFO_CONNECT_TIME>;
	          <median of CURLINFO_STARTTRANSFER_TIME>;
	  		  <median of CURLINFO_TOTAL_TIME>   */
	sprintf(g_prog_output, "SKTEST;%s;%ld;%.6f;%.6f;%.6f;%.6f", 
			ip, response_code, 
			name_lookup_time_median, connect_time_median, 
			start_transfer_time_median, total_time_median);
	//printf("%s \n", g_prog_output);
	
	return RC_OK;
}

/**
* @func   connection_stats_get_statistics
* @desc   String with the statistics according to the following format:
*           TEST;<IP address of HTTP server>;<HTTP response code>;
*             <median of CURLINFO_NAMELOOKUP_TIME>;
*             <median of CURLINFO_CONNECT_TIME>;
*             <median of CURLINFO_STARTTRANSFER_TIME>;
*             <median of CURLINFO_TOTAL_TIME>
*         NOTE: Caller must make sure the first argument has been allocated
*               with at least MAX_SIZE_OF_PROG_OUTPUT
* @param  stat_str    String in the format mentioned at the above desc
* @param  strLen      Len of the returned string
* @return Return Code (taken from RC enum)
*/
RC connection_stats_get_statistics(char* stat_str, size_t* strLen) {
	if (g_prog_output[0] == '\0') {
		printf("ERROR: Result requested before triggereing \n");
		*strLen = 0;
		stat_str = NULL;
		return RC_RESULT_REQUESTED_BEFORE_TRIGGER;
	}
	/* TODO: fix this vulnerability asdsad */ 
	*strLen = strlen(g_prog_output);
	memcpy(stat_str, g_prog_output, *strLen);
	
	return RC_OK;
}

/**
* @desc   Initialize the library (including initialization of libCURL)
* @return Return Code (taken from RC enum)
*/
RC connection_stats_init() {
	CURLcode res;
	
	/* Initialize libCURL easy interface */
	
	/* Global init of curl lib */
	res = curl_global_init(CURL_GLOBAL_DEFAULT);
	if (res != CURLE_OK) {
		printf("connection_stats_init() fail with curl_global_init(): %s\n", 
				curl_easy_strerror(res));
		return RC_ERROR_IN_CURL;
	}

	/* Retrieve CURL handle */
	g_curl = curl_easy_init();
	if (g_curl == NULL) {
		printf("connection_stats_init() fail with curl_easy_init() \n");
		return RC_ERROR_IN_CURL;
	}
	
	/* Open files for traces */
	RC rc = open_trace_files();
	if (rc != RC_OK) {
		printf("connection_stats_init() fail with open_trace_files() \n");
		return rc;
	}
	
	/* Initialize program's output */
	memset(g_prog_output,'\0',sizeof(g_prog_output));
	
	return RC_OK;
}

/**
* @desc   Close the library gracefully (including closing files and libCURL insstance) 
* @return Return Code (taken from RC enum)
*/
RC connection_stats_close() {
#ifdef USE_BODY_HEADER_FILES
	/* close the header file */ 
	fclose(g_header_file);

	/* close the body file */ 
	fclose(g_body_file);
#endif

#ifdef TRACE_ENA
	/* close the trace file */ 
	fclose(g_trace_file);
#endif
	
	/* Cleanup CURL before leaving the program*/
	curl_slist_free_all(g_http_headers_curl_list);
	curl_easy_cleanup(g_curl);
	curl_global_cleanup();
	
	// TODO: does any of the above return RC? use it..
	return RC_OK;
}

/**
* @desc   Add an extra HTTP header to the request
* @param  http_header	HTTP header to be added to CURL request 
*                       (In format: "Header-name: Header-value")
* @return Return Code (taken from RC enum)
*/
RC connection_stats_add_http_hdr(char* http_header) {	
	/* Validate that HTTP Header is legit */
	RC rc = is_valid_http_header(http_header);
	if (rc != RC_OK) {
		return rc;
	}	
	
	/* Append HTTP header to the global list */
	printf("Adding new HTTP header to list: %s \n", http_header);
	g_http_headers_curl_list = 
		curl_slist_append(g_http_headers_curl_list, http_header);
	return RC_OK;
}

/**
* @desc   Trigger for the library to execute HTTP request
*         According to the previously provided arguments.
* @param  http_req_data	Data as received by the user 
* @return Return Code (taken from RC enum)
*/
RC connection_stats_trigger(HttpReqData *p_http_req_data) {
	CURLcode res;
	CurlInfo curl_info_arr[MAX_NUM_OF_SUPPORTED_CURL_OPER];
	
	/* Validate that HTTP data request is legit */
	RC rc = is_valid_http_data_req(p_http_req_data);
	if (rc != RC_OK) {
		return rc;
	}
	
	printf("connection_stats_trigger() called [num_of_http_req=%d, url=%s]\n",
			p_http_req_data->num_of_http_req, p_http_req_data->url);

	/* Initialize program's output */
	memset(g_prog_output,'\0',sizeof(g_prog_output));
	
#ifdef TRACE_ENA
	/* the DEBUGFUNCTION has no effect until we enable VERBOSE */ 
	res = curl_easy_setopt(g_curl, CURLOPT_VERBOSE, 1L);
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_setopt() failed CURLOPT_VERBOSE: %s\n", 
				curl_easy_strerror(res));
		return RC_ERROR_IN_CURL;
	}
#endif
	
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

	/* Set lib CURL option for adding list of previously configured HTTP headers */
	res = curl_easy_setopt(g_curl, CURLOPT_HTTPHEADER, g_http_headers_curl_list);	
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_setopt() failed CURLOPT_HTTPHEADER: %s\n", 
				curl_easy_strerror(res));
		return RC_ERROR_IN_CURL;
	}
	
	struct url_data url_data;
	init_string(&url_data);

#ifdef USE_BODY_HEADER_FILES
	res = curl_easy_setopt(g_curl, CURLOPT_HEADERDATA, g_header_file);
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_setopt() failed CURLOPT_HEADERDATA: %s\n", 
				curl_easy_strerror(res));
		return RC_ERROR_IN_CURL;
	}
	
	//res = curl_easy_setopt(g_curl, CURLOPT_WRITEDATA, &url_data);
	res = curl_easy_setopt(g_curl, CURLOPT_WRITEDATA, g_body_file);
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_setopt() failed CURLOPT_WRITEDATA: %s\n", 
				curl_easy_strerror(res));
		return RC_ERROR_IN_CURL;
	}
#endif  // USE_BODY_HEADER_FILES

	
	//res = curl_easy_setopt(g_curl, CURLOPT_WRITEFUNCTION, write_func);
	/* send all data to this function  */ 
	res = curl_easy_setopt(g_curl, CURLOPT_WRITEFUNCTION, write_data);
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_setopt() failed CURLOPT_WRITEFUNCTION: %s\n", 
				curl_easy_strerror(res));
		return RC_ERROR_IN_CURL;
	}

#ifdef TRACE_ENA
	struct data config;
	config.trace_ascii = 1; /* enable ascii tracing */ 
	res = curl_easy_setopt(g_curl, CURLOPT_DEBUGFUNCTION, trace_func);
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_setopt() failed CURLOPT_DEBUGFUNCTION: %s\n", 
				curl_easy_strerror(res));
		return RC_ERROR_IN_CURL;
	}
	res = curl_easy_setopt(g_curl, CURLOPT_DEBUGDATA, &config);
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_setopt() failed CURLOPT_DEBUGDATA: %s\n", 
				curl_easy_strerror(res));
		return RC_ERROR_IN_CURL;
	}
#endif

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
		connection_stats_collect(&curl_info_arr[i]);
	} // End of FOR loop

	/* Analyze all gathered information - find requested medians
	   Note: This call will also print the program's output */
	connection_stats_analyze(curl_info_arr, p_http_req_data->num_of_http_req);

	//printf("%s\n", url_data.ptr);
	free(url_data.ptr);
	
	return RC_OK;
}

/***********************
** Supporting Methods **
***********************/

#ifdef TRACE_ENA
static void dump(const char *text, FILE *stream, unsigned char *ptr, 
				size_t size, char nohex)
{
	size_t i;
	size_t c;
	
	unsigned int width = 0x10;
	
	if(nohex)
	/* without the hex output, we can fit more on screen */ 
	width = 0x40;
	
	fprintf(stream, "%s, %10.10ld bytes (0x%8.8lx)\n",
			text, (long)size, (long)size);

	for(i = 0; i<size; i += width) {
		fprintf(stream, "%4.4lx: ", (long)i);
		
		if(!nohex) {
			/* hex not disabled, show it */ 
			for(c = 0; c < width; c++)
			if(i + c < size)
				fprintf(stream, "%02x ", ptr[i + c]);
			else
				fputs("   ", stream);
		}
		
		for(c=0; (c < width) && (i + c < size); c++) {
			/* Check for 0D0A; if found, skip past and start a new line of output */ 
			if(nohex && (i + c + 1 < size) && 
			ptr[i + c] == 0x0D && ptr[i + c + 1] == 0x0A) {
				i += (c + 2 - width);
				break;
			}
			fprintf(stream, "%c",
					(ptr[i + c] >= 0x20) && (ptr[i + c]<0x80)?ptr[i + c]:'.');
			/* check again for 0D0A, to avoid an extra \n if it's at width */ 
			if(nohex && (i + c + 2 < size) && 
			ptr[i + c + 1] == 0x0D && ptr[i + c + 2] == 0x0A) {
				i += (c + 3 - width);
				break;
			}
		}
		fputc('\n', stream); /* newline */ 
	}
	fflush(stream);
}
 
static int trace_func(CURL *handle, curl_infotype type, char *data, 
					  size_t size, void *userp)
{
	struct data *config = (struct data *)userp;
	const char *text;
	(void)handle; /* prevent compiler warning */ 
	
	switch(type) {
		case CURLINFO_TEXT:
			//fprintf(stderr, "== Info: %s", data);
			fprintf(g_trace_file, "== Info: %s", data);
			/* FALLTHROUGH */ 
		default: /* in case a new one is introduced to shock us */ 
			return 0;
	
		case CURLINFO_HEADER_OUT:
			text = "=> Send header";
			break;
		case CURLINFO_DATA_OUT:
			text = "=> Send data";
			break;
		case CURLINFO_SSL_DATA_OUT:
			text = "=> Send SSL data";
			break;
		case CURLINFO_HEADER_IN:
			text = "<= Recv header";
			break;
		case CURLINFO_DATA_IN:
			text = "<= Recv data";
			break;
		case CURLINFO_SSL_DATA_IN:
			text = "<= Recv SSL data";
			break;
	}
	
	//dump(text, stderr, (unsigned char *)data, size, config->trace_ascii);
	dump(text, g_trace_file, (unsigned char *)data, size, config->trace_ascii);
	return 0;
}
#endif


static void init_string(struct url_data *url_data) {
	url_data->len = 0;
	url_data->ptr = malloc(url_data->len+1);
	if (url_data->ptr == NULL) {
		fprintf(stderr, "malloc() failed\n");
		exit(EXIT_FAILURE);
	}
	url_data->ptr[0] = '\0';
}
#ifdef WRITEFUNC_USED
static size_t write_func(void *ptr, size_t size, size_t nmemb, struct url_data *url_data)
{
	size_t new_len = url_data->len + size*nmemb;
	url_data->ptr = realloc(url_data->ptr, new_len+1);
	if (url_data->ptr == NULL) {
		fprintf(stderr, "realloc() failed\n");
		exit(EXIT_FAILURE);
	}
	memcpy(url_data->ptr+url_data->len, ptr, size*nmemb);
	url_data->ptr[new_len] = '\0';
	url_data->len = new_len;

	return size*nmemb;
}
#endif // WRITEFUNC_USED

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
	size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
	return written;
}

/*
 * Comparison function between 2 doubles 
 */
static int double_comp(const void* elem1, const void* elem2)
{
    if (*(const double*)elem1 < *(const double*)elem2)
        return -1;
    return *(const double*)elem1 > *(const double*)elem2;
}

/*
 * Get median of given arr (sized arr_size) 
 */
static double get_median(double arr[], int arr_size) {
	double median = 0.0;
	int mid = 0;
	
	if (arr_size<=0) {
		printf("Invalid arr_size = %d \n", arr_size);
		return 0;
	}

	// Sort the array using qsort standart library
	qsort(arr, arr_size, sizeof(double), double_comp);

	// If the array has odd number of elements - median is the middle element
	if((arr_size % 2) != 0)
	{
		mid = (int)(arr_size / 2);
		median = arr[mid];
	}
	else
	{
		// If the array has even number of elements - median is the average
		// of 2 most median
		mid = (arr_size / 2) - 1; // Subtract 1 because array is zero index
		median = (arr[mid] + arr[mid + 1]) / 2;
	}
	
	return median;
}

/*
 * Open the trace files (create trace dir if not opened yet) 
 */
static RC open_trace_files() {
	DIR* dir = opendir("trace");
	if (!dir)
	{
		/* Directory does not exist - create it */
		if (mkdir("trace",0777) == -1) {
			printf("open_trace_files() fail to create trace dir\n");
			return RC_ERROR_IN_FILE_OR_FOLDER;
		}
	}
	
#ifdef USE_BODY_HEADER_FILES
	/* Open the header file */ 
	static const char *header_file_name = "trace/head.out";
	g_header_file = fopen(header_file_name, "wb");
	if(!g_header_file) {
		printf("connection_stats_init() fail to open %s \n", header_file_name);
		curl_easy_cleanup(g_curl);
		return RC_ERROR_IN_FILE_OR_FOLDER;
	}
	
	/* Open the body file */ 
	static const char *body_file_name   = "trace/body.out";
	g_body_file = fopen(body_file_name, "wb");
	if(!g_body_file) {
		printf("connection_stats_init() fail to open %s \n", body_file_name);
		curl_easy_cleanup(g_curl);
		fclose(g_header_file);
		return RC_ERROR_IN_FILE_OR_FOLDER;
	}
#endif  // USE_BODY_HEADER_FILES

#ifdef TRACE_ENA
	/* Open the trace file */ 
	static const char *trace_file_name  = "trace/trace.out";
	g_trace_file = fopen(trace_file_name, "wb");
	if(!g_trace_file) {
		printf("connection_stats_init() fail to open %s \n", trace_file_name);
		curl_easy_cleanup(g_curl);
		#ifdef  USE_BODY_HEADER_FILES
		fclose(g_header_file);
		fclose(g_body_file);
		#endif
		return RC_ERROR_IN_FILE_OR_FOLDER;
	}
#endif
	return RC_OK;
}

/*
 * Validate that HTTP data request is legit 
 */
static RC is_valid_http_data_req(HttpReqData *p_http_req_data) {
	/* Validate num_of_http_req */
	if ((p_http_req_data->num_of_http_req > MAX_NUM_OF_SUPPORTED_CURL_OPER) ||
		(p_http_req_data->num_of_http_req == 0)) {
		printf("Requested number of HTTP requests (%d) must be in range [1:%d] \n", 
				p_http_req_data->num_of_http_req, MAX_NUM_OF_SUPPORTED_CURL_OPER);
		return RC_INVALID_NUM_OF_HTTP_REQ;
	}
	
	/* Validate URL */
	if ((p_http_req_data->url == NULL) || (p_http_req_data->url[0] == '\0') || 
		(strlen(p_http_req_data->url) > URL_MAX_LEN) ||
		(strlen(p_http_req_data->url) < URL_MIN_LEN) ){
		//(memcmp(p_http_req_data->url, "", sizeof(p_http_req_data->url)) == 0)) ||
		/* This validation can be further extendded for WWW prefix, '/', 'http/s' , '.com', etc..*/
		printf("connection_stats_trigger() fail with invalid url %s\n", 
				p_http_req_data->url);
		return RC_INVALID_URL;
	}
	return RC_OK;
}

/*
 * Validate that HTTP Header is legit 
 */
static RC is_valid_http_header(char* http_header) {
	/* Validate HTTP address is legit */
	if ((http_header == NULL) || (http_header[0] == '\0') || 
		(strlen(http_header) > HTTP_HEADER_MAX_LEN) ||
		(strlen(http_header) < HTTP_HEADER_MIN_LEN) ){
		printf("is_valid_http_header() fail with invalid HTTP header %s\n", 
				http_header);
		return RC_INVALID_HTTP_HEADER;
	}
	
	int foundColon = 0;
	char* c = http_header;
	while (*c)
	{
		if (c[0] == ':')
		{
			/* Found ':' char - great */
			foundColon = 1;
		}
		c++;
	}
	if (!foundColon) {
		printf("is_valid_http_header() fail with missing ':' %s\n", 
				http_header);
		return RC_INVALID_HTTP_HEADER;
	}
	return RC_OK;
}
