#ifndef LLM_H
#define LLM_H

#include <stdbool.h>

bool llm_init(void);

void llm_cleanup(void);

bool llm_judge_image(const unsigned char* image_data, int image_size, const char* prompt);

const char* llm_get_last_error(void);

const char* llm_get_last_response(void);

#endif // LLM_H
