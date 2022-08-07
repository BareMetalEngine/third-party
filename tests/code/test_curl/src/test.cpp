/***
* Bare Metal Engine
* Written by Tomasz Jonarski (RexDex)
* Basic middleware testing project
***/

#include "build.h"

extern "C" {
#define CURL_STATICLIB
#include <curl/curl.h>
}

//--

TEST(CURL, BasicRequestTest)
{
    CURL* curl;
    CURLcode res;

    curl = curl_easy_init();
    ASSERT_NE(curl, nullptr);

	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
    curl_easy_setopt(curl, CURLOPT_URL, "https://example.com");

    res = curl_easy_perform(curl);
    ASSERT_EQ(CURLE_OK, res);

    curl_easy_cleanup(curl);
}

//--
