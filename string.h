/**
 * CPE/CSC 159 - Operating System Pragmatics
 * California State University, Sacramento
 * Fall 2020
 *
 * String Utilities
 */

#ifndef STRING_H
#define STRING_H

#ifndef _SIZE_T_
typedef __SIZE_TYPE__ size_t;
#endif

/**
 * Sets the first n bytes pointed to by str to the value specified by c
 *
 * @param   dest - pointer the block of memory to set
 * @param   c    - value to set; passed as an integer but converted to unsigned
 * char when set
 * @param   n    - number of bytes to set
 * @return  pointer to the memory region being set; NULL on error
 */
void *sp_memset(void *dest, int c, size_t n);

/**
 * Copies n bytes from the block of memory pointed to by src to dest
 *
 * @param   dest - pointer to the destination block of memory
 * @param   src  - pointer to the source block of memory
 * @param   n    - number of bytes to read/set
 * @return  pointer to the destination memory region; NULL on error
 */
void *sp_memcpy(void *dest, const void *src, size_t n);

/**
 * Compares the first n bytes of memory pointed to by str1 and str2
 *
 * @param  s1  - pointer to a block of memory
 * @param  s2  - pointer to a block of memory.
 * @param  n   - number of bytes to compare
 * @return  0 if every byte in s1 is equal to s2 or if n is 0
 *         <0 if the first non-matching byte of s1 is less than that of s2
 *         >0 if the first non-matching byte of s1 is greater than that of s2
 *         For a non-zero value the value will indicate the difference
 */
int sp_memcmp(const void *str1, const void *str2, size_t n);

/**
 * Computes the length of the string str up to, but not including the null
 * terminating character
 *
 * @param  str - pointer to the string
 * @return length of the string
 */
size_t sp_strlen(const char *str);

/**
 * Copies the string pointed to by src to the destination dest
 *
 * @param  dest - pointer to the destination string
 * @param  src  - pointer to the source string
 * @return pointer to the destination string
 */
char *sp_strcpy(char *dest, const char *src);

/**
 * Copies up to n characters from the source string src to the destination
 * string dest. If the length of src is less than that of n, the remainder
 * of dest up to n will be filled with NULL characters.
 *
 * @param  dest - pointer to the destination string
 * @param  src  - pointer to the source string
 * @param  n    - maximum number of characters to be copied
 * @return pointer to the destination string
 */
char *sp_strncpy(char *dest, const char *src, size_t n);

/**
 * Compares the string pointed to by str1 to the string pointed to by str2
 *
 * @param  str1 - pointer to the string str1
 * @param  str2 - pointer to the string str2
 * @return  0 if every character in str1 is equal to str2
 *         <0 if the first non-matching character of str1 is less than that of
 * str2 >0 if the first non-matching character of str1 is greater than that of
 * str2 For a non-zero value the value will indicate the difference
 */
int sp_strcmp(const char *str1, const char *str2);

/**
 * Compares the string pointed to by str1 to the string pointed to by str2 up to
 * the first n characters
 *
 * @param  str1 - pointer to the string str1
 * @param  str2 - pointer to the string str2
 * @param  n    - maximum number of characters to be compared
 * @return  0 if every character in str1 is equal to str2 or if n is 0
 *         <0 if the first non-matching character of str1 is less than that of
 * str2 >0 if the first non-matching character of str1 is greater than that of
 * str2 For a non-zero value the value will indicate the difference
 */
int sp_strncmp(const char *str1, const char *str2, size_t n);

#endif
