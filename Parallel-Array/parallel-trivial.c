/**
 * @file parallel-trivial.c
 * @author 오기준 (kijunking@pusan.ac.kr)
 * @brief trivial한 구현 방식을 가진다.
 * @date 2020-04-03
 *
 */
#include "parallel.h"

static int *_id;
static char **_name;
static char **_bban;
static char **_email;

static int _wp = 0;

/**
 * @brief 빈 공간을 가진 Write Pointer(WP) 위치를 찾는다.
 *
 * @return int wp의 index 값으로 만약 빈 공간을 찾지 못하면 -ENOENT를 반환한다.
 */
static int trivial_get_free_wp()
{
        int wp = _wp;
        int is_valid_wp = (wp < MAX_ENTRY_SIZE && wp >= 0);
        if (is_valid_wp && _id[wp] == -1) {
                return wp;
        }

        for (wp = 0; wp < MAX_ENTRY_SIZE; wp++) {
                if (_id[wp] == -1) {
                        return wp;
                }
        }

        return -ENOENT;
}

/**
 * @brief 임의의 id에 해당하는 Write Pointer(WP) 위치를 찾는다.
 *
 * @param id 찾고자하는 id에 해당한다.
 * @return int 찾고자하는 id가 존재하는 WP의 위치를 반환한다. 만약 찾지 못한 경우 -ENOENT를 반환한다.
 */
static int trivial_find_wp(const int id)
{
        int wp;
        for (wp = 0; wp < MAX_ENTRY_SIZE; wp++) {
                if (_id[wp] == id) {
                        return wp;
                }
        }
        return -ENOENT;
}

/**
 * @brief 문자열을 가지는 PA에 값을 넣도록 한다.
 *
 * @param str 현재 쓰고 있는 문자열 값
 * @param arr 쓰고자 하는 PA 포인터
 * @param wp 현재 쓰는 위치
 */
static void trivial_insert_string(char **str, char **arr, const int wp)
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
 * @brief trivial 방식을 초기화 하도록 한다.
 *
 * @return int 정상적인 종료 때에는 0을 비정상 종료의 경우 음수 값을 반환한다.
 */
int trivial_init(void)
{
        int i;

        _id = (int *)malloc(sizeof(int) * MAX_ENTRY_SIZE);
        _name = (char **)malloc(sizeof(char *) * MAX_ENTRY_SIZE);
        _bban = (char **)malloc(sizeof(char *) * MAX_ENTRY_SIZE);
        _email = (char **)malloc(sizeof(char *) * MAX_ENTRY_SIZE);
        if (_id == NULL || _name == NULL || _bban == NULL || _email == NULL) {
                goto exception;
        }

        for (i = 0; i < MAX_ENTRY_SIZE; i++) {
                _id[i] = -1; /** _id[i] = -1; < 빈 공간 정보를 설정한다. */
                _name[i] = (char *)malloc(sizeof(char) * MAX_CHAR_LEN);
                _bban[i] = (char *)malloc(sizeof(char) * MAX_CHAR_LEN);
                _email[i] = (char *)malloc(sizeof(char) * MAX_CHAR_LEN);
                if (_name[i] == NULL || _bban[i] == NULL || _email[i] == NULL) {
                        goto exception;
                }
        }
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
int trivial_insert(char *str, FILE *outp_fp)
{
        int wp, is_valid;
        char *ptr;
        wp = trivial_get_free_wp();
        if (wp < 0) {
#ifdef DEBUG
                fprintf(stderr, "[%s:%s(%d)] Cannot find free WP\n", __FILE__,
                        __FUNCTION__, __LINE__);
#endif
                return -ENOMEM;
        }

        ptr = get_csv_field(&str, ",\n");
        is_valid = (ptr != NULL && strlen(ptr) != 0);
        if (!is_valid) {
#ifdef DEBUG
                fprintf(stderr, "[%s:%s(%d)] Cannot allow the empty \"id\"\n",
                        __FILE__, __FUNCTION__, __LINE__);
#endif
                return -EINVAL;
        }
        _id[wp] = atoi(ptr);

        trivial_insert_string(&str, _name, wp);
        trivial_insert_string(&str, _bban, wp);
        trivial_insert_string(&str, _email, wp);

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
int trivial_search(char *str, FILE *outp_fp)
{
        int wp, id;
        id = atoi(get_csv_field(&str, ","));
        wp = trivial_find_wp(id);
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
int trivial_remove(char *str, FILE *outp_fp)
{
        int wp, id;
        id = atoi(get_csv_field(&str, ",\n"));
        wp = trivial_find_wp(id);
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
int trivial_get_current_usage()
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
 * @brief trivial에서 설정된 것들을 해제한다.
 *
 */
void trivial_free(void)
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
