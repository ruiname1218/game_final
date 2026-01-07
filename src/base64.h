#ifndef BASE64_H
#define BASE64_H

#include <stddef.h>

// Base64エンコード
// 戻り値: エンコードされた文字列（呼び出し側でfree必要）
char* base64_encode(const unsigned char* data, size_t input_length, size_t* output_length);

#endif // BASE64_H
