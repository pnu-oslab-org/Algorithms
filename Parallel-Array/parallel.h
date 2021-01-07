/**
 * @file parallel.h
 * @author 오기준 (kijunking@pusan.ac.kr)
 * @brief PA(Parallel Array)에서 사용되는 모든 함수에 대한 선언이 적혀있다.
 * @date 2020-04-03
 *
 */

#ifndef PARALLEL_H_
#define PARALLEL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#define MAX_CHAR_LEN                                                           \
        64 /** MAX_CHAR_LEN < 레코드에서 값을 받을j수 있는 최대 크기 */
#define MAX_ENTRY_SIZE 10000 /** MAX_ENTRY_SIZE < 기본 값은 10000 */
#define NR_ITEMS 4
#define MAX_LINE_LEN (MAX_CHAR_LEN * NR_ITEMS)

//#define TRIVIAL /** TRIVIAL < 이것을 enable하면 TRIVIAL한 버전이 실행된다. */
//#define DEBUG /** DEBUG < 이것을 enable하면 DEBUG 로그가 찍히게 된다. */

#define FAIL_TO_SEARCH -1

/**
 * @brief trivial, improve의 함수를 선택할 수 있는 구조체이다.
 *
 */
struct op {
        int (*init)(void);
        int (*insert)(
                char *str,
                FILE *outp_fp); /** int (*insert)(char *str, FILE *outp_fp); < PA에 insert를 수행한다. */
        int (*search)(
                char *str,
                FILE *outp_fp); /** int (*search)(char *str, FILE *outp_fp); < PA에 search 수행, search도 겸한다. */
        int (*remove)(
                char *str,
                FILE *outp_fp); /** int (*remove)(char *str, FILE *outp_fp); < PA에 remove를 수행한다. */
        void (*free)(void);
        int (*get_current_usage)(
                void); /** int (*get_current_usage)(void); < PA의 현재 메모리 사용량을 보여준다.(DEBUG 전용) */
};

#ifdef TRIVIAL
int trivial_init(void);
int trivial_insert(char *str, FILE *outp_fp);
int trivial_search(char *str, FILE *outp_fp);
int trivial_remove(char *str, FILE *outp_fp);
int trivial_get_current_usage();
void trivial_free(void);
#else
int improve_init(void);
int improve_insert(char *str, FILE *outp_fp);
int improve_search(char *str, FILE *outp_fp);
int improve_remove(char *str, FILE *outp_fp);
int improve_get_current_usage();
void improve_free(void);
#endif

/**
 * @brief CSV 값을 하나하나 읽어오도록 한다.
 * @warning 원본이 수정되므로 유의해야 한다.
 *
 * @param strptr 현재 field를 가져오고자 하는 CSV 행
 * @param delim field를 나누는 기준 값
 * @return char* field에 해당하는 포인터
 */
static inline char *get_csv_field(char **strptr, const char *delim)
{
        char *ptr = *strptr;

        if (ptr == NULL) {
                return NULL;
        }

        while (**strptr) {
                if (strchr(delim, **strptr) != NULL) {
                        **strptr = 0x00;
                        (*strptr)++;
                        return ptr;
                }
                (*strptr)++;
        }
        *strptr = NULL;

        return ptr;
}

#endif
