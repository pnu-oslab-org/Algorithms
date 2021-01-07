/**
 * @file parallel-improve.c
 * @author 오기준 (kijunking@pusan.ac.kr)
 * @brief improve한 구현 방식을 가진다.
 * @date 2020-04-03
 *
 */
#include "parallel.h"

#define BITS_PER_BYTE                                                          \
        8 /** BITS_PER_BYTE < 1 바이트에 들어가는 비트 수를 지칭한다. */
#define BITMAP_LEN                                                             \
        (sizeof(unsigned long) *                                               \
         BITS_PER_BYTE) /** BITMAP_LEN < 비트맵의 길이를 가진다. */
#define BITMAP_FULL                                                            \
        0xffffffff /** BITMAP_FULL < unsigned long이 4 바이트 이므로 이를 마스킹을 할 수 있도록 만든다. */

static int *_id;
static char **_name;
static char **_bban;
static char **_email;

static unsigned long *_bitmap;

static int _wp = 0;

/**
 * @brief 빈 공간을 가진 Write Pointer(WP) 위치를 찾는다.
 *
 * @return int wp의 index 값으로 만약 빈 공간을 찾지 못하면 -ENOENT를 반환한다.
 */
static int improve_get_free_wp(int id)
{
        int wp = _wp, i = 0, offset = 0;
        int is_valid_wp = (wp < MAX_ENTRY_SIZE && wp >= 0);
        if (is_valid_wp && _id[wp] == -1) {
                i = wp / BITMAP_LEN;
                offset = wp % BITMAP_LEN;
                _bitmap[i] |=
                        (0x1
                         << offset); /**_bitmap[i] |= (0x1 << offset); < 비트맵에 값을 설정한다. */
                return wp;
        }

        /**
         * @brief 4 byte 단위로 비트맵을 읽어서 꽉 차지 않은 구역을 찾도록 한다.
         *
         */
        for (i = 0; i * BITMAP_LEN < MAX_ENTRY_SIZE; i++) {
                if (_bitmap[i] < BITMAP_FULL) {
                        wp = i * BITMAP_LEN;
                        for (offset = 0; offset < BITMAP_LEN; offset++) {
                                if (wp + offset >= MAX_ENTRY_SIZE) {
                                        goto ret;
                                }
                                if (_id[wp + offset] == -1) {
                                        _bitmap[i] |= (0x1 << offset);
                                        return wp + offset;
                                }
                        } // end of for
                } // end of if
        } // end of for
ret:
        return -ENOENT;
}

/**
 * @brief 임의의 id에 해당하는 Write Pointer(WP) 위치를 찾는다.
 *
 * @param id 찾고자하는 id에 해당한다.
 * @param is_remove remove 명령인 경우에 해당하는 지를 확인한다.
 * @return int 찾고자하는 id가 존재하는 WP의 위치를 반환한다. 만약 찾지 못한 경우 -ENOENT를 반환한다.
 */
static int improve_find_wp(const int id, const int is_remove)
{
        int wp, i, offset;

        /**
        * @brief is_remove의 경우에는 bitmap unset을 하는 과정을 추가한다.
        *
        */
        for (i = 0; i * BITMAP_LEN < MAX_ENTRY_SIZE; i++) {
                if (_bitmap[i] > 0x0) {
                        wp = i * BITMAP_LEN;
                        for (offset = 0; offset < BITMAP_LEN; offset++) {
                                if (wp + offset >= MAX_ENTRY_SIZE) {
                                        goto ret;
                                }
                                if (_id[wp + offset] == id) {
                                        if (is_remove) {
                                                _bitmap[i] &= ~(0x1 << offset);
                                        }
                                        return (wp + offset);
                                }
                        } // end of for
                } // end of if
        } // end of for
ret:
        return -ENOENT;
}

/**
 * @brief 문자열을 가지는 PA에 값을 넣도록 한다.
 *
 * @param str 현재 쓰고 있는 문자열 값
 * @param arr 쓰고자 하는 PA 포인터
 * @param wp 현재 쓰는 위치
 */
static void improve_insert_string(char **str, char **arr, const int wp)
{
        int is_valid;
        char *ptr;
        ptr = get_csv_field(str, ",\n");
        is_valid = (ptr != NULL && strlen(ptr) != 0);
#ifdef DEBUG
        if (!is_valid) {
                fprintf(stderr, "[%s:%s(%d)] Cannot find field value(id: %d)\n",
                        __FILE__, __FUNCTION__, __LINE__, _id[wp]);
        }
#endif
        strncpy(arr[wp], (is_valid ? ptr : "<EMPTY>"), MAX_CHAR_LEN);
}

/**
 * @brief improve 방식을 초기화 하도록 한다.
 *
 * @return int 정상적인 경우에는 0을 그렇지 않고 메모리를 할당 받지 못하는 등의 예외가 발생하면 음수 값을 반환한다.
 */
int improve_init(void)
{
        int i, nr_bitmap;
        int total_size = 0;

        _id = (int *)malloc(sizeof(int) * MAX_ENTRY_SIZE);
        total_size += sizeof(int) * MAX_ENTRY_SIZE;
        _name = (char **)malloc(sizeof(char *) * MAX_ENTRY_SIZE);
        _bban = (char **)malloc(sizeof(char *) * MAX_ENTRY_SIZE);
        _email = (char **)malloc(sizeof(char *) * MAX_ENTRY_SIZE);
        total_size += sizeof(char *) * MAX_ENTRY_SIZE * 3;
        if (_id == NULL || _name == NULL || _bban == NULL || _email == NULL) {
                goto exception;
        }

        for (i = 0; i < MAX_ENTRY_SIZE; i++) {
                _id[i] = -1; /**_id[i] = -1; < 빈 공간 정보를 설정한다. */
                _name[i] = (char *)malloc(sizeof(char) * MAX_CHAR_LEN);
                _bban[i] = (char *)malloc(sizeof(char) * MAX_CHAR_LEN);
                _email[i] = (char *)malloc(sizeof(char) * MAX_CHAR_LEN);
                total_size += sizeof(char) * MAX_CHAR_LEN * 3;
                if (_name[i] == NULL || _bban[i] == NULL || _email[i] == NULL) {
                        goto exception;
                }
        }

        nr_bitmap = MAX_ENTRY_SIZE / BITMAP_LEN;
        _bitmap = (unsigned long *)malloc(sizeof(unsigned long) *
                                          (nr_bitmap + 1));
        if (_bitmap == NULL) {
                goto exception;
        }
        printf("[%s:%s(%d)] Additional Memory: %.2lfKB/%.2lfKB(%lf%%)\n",
               __FILE__, __FUNCTION__, __LINE__,
               (sizeof(unsigned long) * (nr_bitmap + 1)) / 1000.0,
               total_size / 1000.0,
               (double)(sizeof(unsigned long) * (nr_bitmap + 1)) / total_size *
                       100);
#ifdef DEBUG
        for (i = 0; i * BITMAP_LEN < MAX_ENTRY_SIZE; i++) {
                _bitmap[i] &= 0x00;
                printf("_bitmap[%d]: 0x%08lx\n", i, _bitmap[i]);
        }
#endif
        return 0;

exception:
#ifdef DEBUG
        fprintf(stderr, "[%s:%s(%d)] Cannot allocate the MEMORY\n", __FILE__,
                __FUNCTION__, __LINE__);
#endif
        return -ENOMEM;
}

/**
 * @brief PA에 값을 집어넣도록 한다.
 *
 * @param str COMMAND를 제외한 csv 형태의 행을 의미한다.
 * @param outp_fp 출력을 하는 파일에 대한 포인터이다.
 *
 * @return int 정상적인 종료 때에는 0을 비정상 종료의 경우 음수 값을 반환한다.
 */
int improve_insert(char *str, FILE *outp_fp)
{
        int wp, is_valid, id;
        char *ptr;

        ptr = get_csv_field(&str, ",\n");
        is_valid = (ptr != NULL && strlen(ptr) != 0);
        if (!is_valid) {
#ifdef DEBUG
                fprintf(stderr, "[%s:%s(%d)] Cannot allow the empty \"id\"\n",
                        __FILE__, __FUNCTION__, __LINE__);
#endif
                return -EINVAL;
        }
        id = atoi(ptr);
        wp = improve_get_free_wp(id);
        if (wp < 0) {
#ifdef DEBUG
                fprintf(stderr, "[%s:%s(%d)] Cannot find free WP\n", __FILE__,
                        __FUNCTION__, __LINE__);
#endif
                return -ENOMEM;
        }

        _id[wp] = id;
        improve_insert_string(&str, _name, wp);
        improve_insert_string(&str, _bban, wp);
        improve_insert_string(&str, _email, wp);

#ifdef DEBUG
        fprintf(outp_fp, "INSERT\t%d\t%s\t%s\t%s\n", _id[wp], _name[wp],
                _bban[wp], _email[wp]);
#else
        fprintf(outp_fp, "INSERT\t%d\n", _id[wp]);
#endif
        _wp = wp + 1;
        return 0;
}

/**
 * @brief id에 기반하여 PA에 존재하는 값을 찾는 함수에 해당한다.
 *
 * @param str COMMAND를 제외한 csv 형태의 행을 의미한다.
 * @param outp_fp 출력하는 파일에 대한 포인터이다.
 *
 * @return int 정상 종료 시에 0을 반환하고 비정상 종료시 -1을 반환한다.
 */
int improve_search(char *str, FILE *outp_fp)
{
        int wp, id;
        id = atoi(get_csv_field(&str, ","));
        wp = improve_find_wp(id, 0);
        if (wp < 0) {
#ifdef DEBUG
                fprintf(stderr, "[%s:%s(%d)] Cannot find WP\n", __FILE__,
                        __FUNCTION__, __LINE__);
#endif
                return -ENOMEM;
        }
        fprintf(outp_fp, "SEARCH\t%d\t%s\t%s\t%s\n", _id[wp], _name[wp],
                _bban[wp], _email[wp]);
        return 0;
}

/**
 * @brief id에 기반하여 PA에 해당하는 값을 제거한다.
 *
 * @param str COMMAND를 제외한 csv 형태의 행을 의미한다.
 * @param outp_fp 출력하는 파일에 대한 포인터이다.
 *
 * @return int 정상 종료 시에 0을 반환하고 비정상 종료시 -1을 반환한다.
 */
int improve_remove(char *str, FILE *outp_fp)
{
        int wp, id;
        id = atoi(get_csv_field(&str, ",\n"));
        wp = improve_find_wp(id, 1);
        if (wp < 0) {
#ifdef DEBUG
                fprintf(stderr, "[%s:%s(%d)] Cannot find WP\n", __FILE__,
                        __FUNCTION__, __LINE__);
#endif
                return -ENOMEM;
        }
        _id[wp] = -1;
        fprintf(outp_fp, "REMOVE\t%d\n", id);
        return 0;
}

/**
 * @brief 현재 사용량을 출력하도록 한다.
 *
 * @return int 현재 사용량을 출력한다.
 */
int improve_get_current_usage()
{
        int wp, count = 0;
        for (wp = 0; wp < MAX_ENTRY_SIZE; wp++) {
                if (_id[wp] != -1) {
                        count++;
                }
        }
        return count;
}

/**
 * @brief improve에서 설정된 것들을 해제한다.
 *
 */
void improve_free(void)
{
        int i;

        free(_id);
        for (i = 0; i < MAX_ENTRY_SIZE; i++) {
                free(_name[i]);
                free(_bban[i]);
                free(_email[i]);
        }
        free(_name);
        free(_bban);
        free(_email);
}
