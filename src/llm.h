#ifndef LLM_H
#define LLM_H

#include <stdbool.h>

// LLM initialization
bool llm_init(void);

// LLM cleanup
void llm_cleanup(void);

// Judge image - returns true if correct
bool llm_judge_image(const unsigned char* image_data, int image_size, const char* prompt);

// Get last error message
const char* llm_get_last_error(void);

// Get last AI response text (for display)
const char* llm_get_last_response(void);

#endif // LLM_H
