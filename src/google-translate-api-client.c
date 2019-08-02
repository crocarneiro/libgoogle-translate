# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <curl/curl.h>
# include "cJSON.h"
# include "google-translate-api-client.h"

# define URL_WITH_KEY ""
# define DONT_VERIFY_PEER_SSL 0

CURL *curl;

struct string
{
    char *ptr;
    int len;
};

void init_string(struct string *s);
char *build_json(char *text, char *from, char *to);
size_t write_callback(char *ptr, size_t size, size_t nmemb, struct string *s);
char *get_translated_text_from_response(char *response);

void init_client()
{
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
}

void cleanup_client()
{
    curl_easy_cleanup(curl);
}

char *translate_text(char *text, char *from, char *to)
{
    struct curl_slist *chunk = NULL;
    struct string response;

    CURLcode res = CURLE_FAILED_INIT;

    init_string(&response);

    chunk = curl_slist_append(chunk, "Content-Type:application/json");
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, DONT_VERIFY_PEER_SSL);
    curl_easy_setopt(curl, CURLOPT_URL, URL_WITH_KEY);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, build_json(text, from, to));
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (char*)&response);

    res = curl_easy_perform(curl);
    if(res != CURLE_OK)
        fprintf(stderr, "%s", curl_easy_strerror(res));

    curl_slist_free_all(chunk);

    return get_translated_text_from_response(response.ptr);
}

void init_string(struct string *s)
{
    s->len = 0;
    s->ptr = malloc(1);

    if(s->ptr == NULL) {
        fprintf(stderr, "malloc() failed.\n");
        exit(1);
    }

    s->ptr[0] = '\0';
}

char *build_json(char *text, char *from, char *to)
{
    char *json = NULL;
    cJSON *root = NULL;
    cJSON *q = NULL;
    cJSON *source = NULL;
    cJSON *target = NULL;

    root = cJSON_CreateObject();
    q = cJSON_CreateString(text);
    source = cJSON_CreateString(from);
    target = cJSON_CreateString(to);

    cJSON_AddItemToObject(root, "q", q);
    cJSON_AddItemToObject(root, "source", source);
    cJSON_AddItemToObject(root, "target", target);

    json = cJSON_Print(root);
    cJSON_free(root);

    return json;
}

size_t write_callback(char *ptr, size_t size, size_t nmemb, struct string *s)
{
    size_t new_len = s->len + size*nmemb;
    s->ptr = realloc(s->ptr, new_len+1);

    if (s->ptr == NULL) {
      fprintf(stderr, "realloc() failed\n");
      exit(1);
    }

    memcpy(s->ptr+s->len, ptr, size*nmemb);
    s->ptr[new_len] = '\0';
    s->len = new_len;

    return size*nmemb;
}

char *get_translated_text_from_response(char *response)
{
    cJSON *root = cJSON_Parse(response);
    cJSON *data = cJSON_GetObjectItem(root, "data");
    cJSON *translations = cJSON_GetObjectItem(data, "translations");
    cJSON *translatedText = cJSON_GetArrayItem(translations, 0);

    return (char *)cJSON_GetObjectItem(translatedText, "translatedText")->valuestring;
}