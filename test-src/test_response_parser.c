#include "test_util.h"

#include <string.h>

#include "wsfs/response_parser.h"

static void wsfs_response_parse_str(
	char const * buffer,
	struct wsfs_response * response)
{
	size_t length = strlen(buffer);
	wsfs_response_parse(buffer, length, response);
}

void test_request_parser()
{
	struct wsfs_response response;

	// invalid json
	wsfs_response_parse_str("", &response);
	ASSERT_NE(WSFS_GOOD, response.status);
	ASSERT_EQ(-1, response.id);
	ASSERT_EQ(NULL, response.result);

	// invalid json
	wsfs_response_parse_str("invalid_json", &response);
	ASSERT_NE(WSFS_GOOD, response.status);
	ASSERT_EQ(-1, response.id);
	ASSERT_EQ(NULL, response.result);

	// no object
	wsfs_response_parse_str("[]", &response);
	ASSERT_NE(WSFS_GOOD, response.status);
	ASSERT_EQ(-1, response.id);
	ASSERT_EQ(NULL, response.result);

	// empty
	wsfs_response_parse_str("{}", &response);
	ASSERT_NE(WSFS_GOOD, response.status);
	ASSERT_EQ(-1, response.id);
	ASSERT_EQ(NULL, response.result);

	// no data
	wsfs_response_parse_str("{\"id\":42}", &response);
	ASSERT_NE(WSFS_GOOD, response.status);
	ASSERT_EQ(42, response.id);
	ASSERT_EQ(NULL, response.result);

	// custom error code
	wsfs_response_parse_str("{\"error\":{\"code\": 42}, \"id\": 42}", &response);
	ASSERT_NE(WSFS_GOOD, response.status);
	ASSERT_EQ(42, response.status);
	ASSERT_EQ(42, response.id);
	ASSERT_EQ(NULL, response.result);

	// valid response
	wsfs_response_parse_str("{\"result\": true, \"id\": 42}", &response);
	ASSERT_EQ(WSFS_GOOD, response.status);
	ASSERT_EQ(42, response.id);
	ASSERT_NE(NULL, response.result);
	json_decref(response.result);
}