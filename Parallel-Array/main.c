/**
 * @file main.c
 * @author 오기준 (kijunking@pusan.ac.kr)
 * @brief 자료구조에 대한 테스트가 수행되는 파일이다.
 * @details 이 프로그램을 사용하는 방법은 해당 소스 코드가 있는 폴더에
 테스트 케이스 파일을 가져와서(e.g. uniform.inp) 이름을 `test.inp`로 변경하고 넣도록 한다.
 입력 파일은 상수로 설정되어 있으므로 반드시 입력 파일의 이름은 `test.inp`로 해주도록 한다.

 그리고 만약 내가 trivial한 동작을 돌리고 싶은 경우에는 parallel.h 파일의 `#define TRIVIAL`
 매크로의 주석을 해제해주고, 그렇지 않은 개선된 버전을 동작시키는 경우에는 해당 주석을 제거해주도록 한다.
 단, DEBUG 주석은 해제하지 말도록 한다.

 여기까지 했으면 code block의 **Build and run**을 클릭하여 실행해주도록 한다.
 * @date 2020-04-03
 *
 */
#include "parallel.h"

static FILE *inp_fp; /**< 입력 파일의 포인터이다. */
static FILE *outp_fp; /**< 출력 파일의 포인터이다. */
static int N; /**< 입력 파일에 있는 명령의 갯수를 지칭한다. */
static struct op operation; /**< trivial, improve를 선택할 수 있도록 해준다. */

/**
 * @brief 입력 파일 및 출력 파일, N 값을 설정한다. 그리고 PA를 동적 할당하고, 빈 공간 정보를 설정한다.
 *
 * @param inp_file 입력 파일의 이름을 가진다.
 * @param outp_file 출력 파일의 이름을 가진다.
 */
static void setup(const char *inp_file, const char *outp_file)
{
        int ret;

        inp_fp = fopen(inp_file, "r");
        outp_fp = fopen(outp_file, "w");

        fscanf(inp_fp, "%d\n", &N);

        if (N < 0) {
                fprintf(stderr, "[%s:%s,(%d)] invalid N value(%d)", __FILE__,
                        __FUNCTION__, __LINE__, N);
                exit(-EINVAL);
        }

#ifdef TRIVIAL
        printf("[%s:%s(%d)] trivial setting initialize\n", __FILE__,
               __FUNCTION__, __LINE__);
        operation.init = trivial_init;
        operation.insert = trivial_insert;
        operation.search = trivial_search;
        operation.remove = trivial_remove;
#ifdef DEBUG
        operation.get_current_usage = trivial_get_current_usage;
#endif
        operation.free = trivial_free;
#else
        printf("[%s:%s(%d)] improve setting initialize\n", __FILE__,
               __FUNCTION__, __LINE__);
        operation.init = improve_init;
        operation.insert = improve_insert;
        operation.search = improve_search;
        operation.remove = improve_remove;
#ifdef DEBUG
        operation.get_current_usage = improve_get_current_usage;
#endif
        operation.free = improve_free;
#endif
        ret = operation.init();
        if (ret != 0) {
                fprintf(stderr, "[%s:%s(%d)] Initialize failed\n", __FILE__,
                        __FUNCTION__, __LINE__);
                exit(ret);
        }
}

/**
 * @brief 실제 자료구조를 테스트하는 부분이다.
 *
 */
static void run(clock_t start)
{
        int i = 0;
        char *command, *temp;
        const int _10_percent = N / 10;
        char *line = (char *)malloc(sizeof(char) * MAX_LINE_LEN);

        for (i = 0; i < N; i++) {
                temp = line;
                if (i % _10_percent == 0) {
                        printf("[%s:%s(%d)] %.2lf%% done (%.2lfs)\n", __FILE__,
                               __FUNCTION__, __LINE__, (double)i / N * 100,
                               (double)(clock() - start) / CLOCKS_PER_SEC);
                }
                fgets(temp, MAX_LINE_LEN, inp_fp);
                command = get_csv_field(&temp, ",");
                if (!strcmp(command, "INSERT")) {
                        if (operation.insert(temp, outp_fp)) {
                                fprintf(outp_fp, "INSERT <FAIL>\n");
                        }
                } else if (!strcmp(command, "SEARCH")) {
                        if (operation.search(temp, outp_fp)) {
                                fprintf(outp_fp, "SEARCH <FAIL>\n");
                        }
                } else if (!strcmp(command, "REMOVE")) {
                        if (operation.remove(temp, outp_fp)) {
                                fprintf(outp_fp, "REMOVE <FAIL>\n");
                        }
                } else {
#ifdef DEBUG
                        fprintf(stderr, "[%s:%s(%d)] invalid command detected",
                                __FILE__, __FUNCTION__, __LINE__);
#endif
                        fprintf(outp_fp, "OTHERS <FAIL>\n");
                } // end of if

#ifdef DEBUG
                static int max = -1;
                if (max < operation.get_current_usage()) {
                        printf("max usage: %d/%d\n", max, MAX_ENTRY_SIZE);
                        max = operation.get_current_usage();
                }
#endif
        }

        free(line);
}

/**
 * @brief 파일을 닫고, 동적 할당된 PA를 해제해주도록 한다.
 *
 */
static void close()
{
        operation.free();
        fclose(inp_fp);
        fclose(outp_fp);
}

int main(void)
{
        clock_t start;

        setup("test.inp", "test.out");
        start = clock();
        run(start);
        printf("[%s:%s(%d)] total execution time %.2lfs", __FILE__,
               __FUNCTION__, __LINE__,
               (double)(clock() - start) / CLOCKS_PER_SEC);
        close();
        return 0;
}
