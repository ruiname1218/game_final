#include "llm.h"
#include "base64.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "cJSON.h"

#define GEMINI_API_URL "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.5-flash:generateContent"
#define MAX_ERROR_LEN 256

static char api_key[256] = {0};
static char last_error[MAX_ERROR_LEN] = {0};
static char last_response[1024] = {0};  // Store full AI response
static CURL* curl = NULL;

// Response data structure
typedef struct {
    char* data;
    size_t size;
} ResponseData;

static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    ResponseData* resp = (ResponseData*)userp;
    
    char* ptr = realloc(resp->data, resp->size + realsize + 1);
    if (ptr == NULL) {
        return 0;
    }
    
    resp->data = ptr;
    memcpy(&(resp->data[resp->size]), contents, realsize);
    resp->size += realsize;
    resp->data[resp->size] = '\0';
    
    return realsize;
}

bool llm_init(void) {
    const char* key = getenv("GEMINI_API_KEY");
    if (key == NULL || strlen(key) == 0) {
        snprintf(last_error, MAX_ERROR_LEN, "GEMINI_API_KEY environment variable not set");
        return false;
    }
    strncpy(api_key, key, sizeof(api_key) - 1);
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl == NULL) {
        snprintf(last_error, MAX_ERROR_LEN, "Failed to initialize CURL");
        return false;
    }
    
    return true;
}

void llm_cleanup(void) {
    if (curl) {
        curl_easy_cleanup(curl);
        curl = NULL;
    }
    curl_global_cleanup();
}

bool llm_judge_image(const unsigned char* image_data, int image_size, const char* prompt) {
    if (curl == NULL) {
        snprintf(last_error, MAX_ERROR_LEN, "LLM not initialized");
        return false;
    }
    
    size_t b64_len = 0;
    char* b64_data = base64_encode(image_data, image_size, &b64_len);
    if (b64_data == NULL) {
        snprintf(last_error, MAX_ERROR_LEN, "Failed to encode image to base64");
        return false;
    }
    
    cJSON* root = cJSON_CreateObject();
    cJSON* contents = cJSON_CreateArray();
    cJSON* content = cJSON_CreateObject();
    cJSON* parts = cJSON_CreateArray();
    
    cJSON* image_part = cJSON_CreateObject();
    cJSON* inline_data = cJSON_CreateObject();
    cJSON_AddStringToObject(inline_data, "mime_type", "image/png");
    cJSON_AddStringToObject(inline_data, "data", b64_data);
    cJSON_AddItemToObject(image_part, "inline_data", inline_data);
    cJSON_AddItemToArray(parts, image_part);
    
    cJSON* text_part = cJSON_CreateObject();
    char prompt_text[512];
    snprintf(prompt_text, sizeof(prompt_text), 
        "Is this hand-drawn sketch a drawing of '%s'? "
        "Judge if the drawing looks like '%s'. "
        "Even simplified or rough sketches are correct if they capture the key features. "
        "Answer with YES or NO as the first word, then briefly explain.", 
        prompt, prompt);
    cJSON_AddStringToObject(text_part, "text", prompt_text);
    cJSON_AddItemToArray(parts, text_part);
    
    cJSON_AddItemToObject(content, "parts", parts);
    cJSON_AddItemToArray(contents, content);
    cJSON_AddItemToObject(root, "contents", contents);
    
    char* json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    free(b64_data);
    
    if (json_str == NULL) {
        snprintf(last_error, MAX_ERROR_LEN, "Failed to create JSON request");
        return false;
    }
    
    char url[512];
    snprintf(url, sizeof(url), "%s?key=%s", GEMINI_API_URL, api_key);
    
    ResponseData response = {0};
    response.data = malloc(1);
    response.size = 0;
    
    curl_easy_reset(curl);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_str);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    #ifdef DEBUG
    printf("DEBUG: Sending request to Gemini API...\n");
    #endif
    
    CURLcode res = curl_easy_perform(curl);
    
    curl_slist_free_all(headers);
    free(json_str);
    
    if (res != CURLE_OK) {
        snprintf(last_error, MAX_ERROR_LEN, "CURL error: %s", curl_easy_strerror(res));
        #ifdef DEBUG
        printf("DEBUG: CURL failed: %s\n", curl_easy_strerror(res));
        #endif
        free(response.data);
        return false;
    }
    
    #ifdef DEBUG
    printf("DEBUG: Response received, size=%zu\n", response.size);
    printf("DEBUG: Response data: %.500s\n", response.data);
    #endif
    
    cJSON* resp_json = cJSON_Parse(response.data);
    free(response.data);
    
    if (resp_json == NULL) {
        snprintf(last_error, MAX_ERROR_LEN, "Failed to parse response JSON");
        return false;
    }
    
    cJSON* error = cJSON_GetObjectItem(resp_json, "error");
    if (error != NULL) {
        cJSON* message = cJSON_GetObjectItem(error, "message");
        if (message && message->valuestring) {
            snprintf(last_error, MAX_ERROR_LEN, "API error: %s", message->valuestring);
        } else {
            snprintf(last_error, MAX_ERROR_LEN, "Unknown API error");
        }
        cJSON_Delete(resp_json);
        return false;
    }
    
    cJSON* candidates = cJSON_GetObjectItem(resp_json, "candidates");
    if (candidates == NULL || !cJSON_IsArray(candidates) || cJSON_GetArraySize(candidates) == 0) {
        snprintf(last_error, MAX_ERROR_LEN, "No candidates in response");
        cJSON_Delete(resp_json);
        return false;
    }
    
    cJSON* candidate = cJSON_GetArrayItem(candidates, 0);
    cJSON* content_obj = cJSON_GetObjectItem(candidate, "content");
    cJSON* parts_arr = cJSON_GetObjectItem(content_obj, "parts");
    
    if (parts_arr == NULL || !cJSON_IsArray(parts_arr) || cJSON_GetArraySize(parts_arr) == 0) {
        snprintf(last_error, MAX_ERROR_LEN, "No parts in response");
        cJSON_Delete(resp_json);
        return false;
    }
    
    cJSON* first_part = cJSON_GetArrayItem(parts_arr, 0);
    cJSON* text = cJSON_GetObjectItem(first_part, "text");
    
    if (text == NULL || text->valuestring == NULL) {
        snprintf(last_error, MAX_ERROR_LEN, "No text in response");
        cJSON_Delete(resp_json);
        return false;
    }
    
    const char* response_text = text->valuestring;
    bool result = false;
    
    strncpy(last_response, response_text, sizeof(last_response) - 1);
    
    while (*response_text == ' ' || *response_text == '\n' || *response_text == '\t') {
        response_text++;
    }
    
    if (strncasecmp(response_text, "YES", 3) == 0) {
        char next = response_text[3];
        if (next == '\0' || next == ' ' || next == ',' || next == '.' || next == '!' || next == '\n') {
            result = true;
        }
    }
    
    #ifdef DEBUG
    printf("LLM Response: %s\n", last_response);
    printf("Judgment: %s\n", result ? "CORRECT" : "INCORRECT");
    #endif
    
    cJSON_Delete(resp_json);
    return result;
}

const char* llm_get_last_error(void) {
    return last_error;
}

const char* llm_get_last_response(void) {
    return last_response;
}
