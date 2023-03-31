/*
 * Copyright(c) 2023 All rights reserved by Heekuck Oh.
 * 이 프로그램은 한양대학교 ERICA 컴퓨터학부 학생을 위한 교육용으로 제작되었다.
 * 한양대학교 ERICA 학생이 아닌 이는 프로그램을 수정하거나 배포할 수 없다.
 * 프로그램을 수정할 경우 날짜, 학과, 학번, 이름, 수정 내용을 기록한다.
 *
 * [수정사항] - 수정자: 한양대 ERICA 컴퓨터학부 2019063363 최수용
 * 2023/03/18 redirection(),do_pipe() 함수 추가 by 한양대 ERICA 컴퓨터학부 2019063363 최수용
 * 2023/03/20 cmd_exec함수안의 내용을 모듈화하여 함수단위로 구분 by 한양대 ERICA 컴퓨터학부 2019063363 최수용
 * 2023/03/22 주석추가 by 한양대 ERICA 컴퓨터학부 2019063363 최수용
 * 2023/03/24 오류처리 by 한양대 ERICA 컴퓨터학부 2019063363 최수용
 * 2023/03/25 함수 및 변수 이름 카멜케이스 -> 스네이크케이스,오류처리 부분에 fflush(stdout) 추가 by 한양대 ERICA 컴퓨터학부 2019063363 최수용
 * 2023/03/26 for문에서 argv[i] != NULL로 수정 by 한양대 ERICA 컴퓨터학부 2019063363 최수용
 * 2023/03/29 WRITE_END,READ_END 추가 by 한양대 ERICA 컴퓨터학부 2019063363 최수용
 * 2023/03/30 주석 문장 다듬기 by 한양대 ERICA 컴퓨터학부 2019063363 최수용
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_LINE 80 /* 명령어의 최대 길이 */
#define WRITE_END 1 /* pipe(fd) fd[1] 부분 가독성을 위해 추가 */
#define READ_END 0 /* pipe(fd) fd[0] 부분 가독성을 위해 추가 */


/*
* for문을 통해 "<",">" 기호를 찾고,
* 리다이렉션을 수행하는 함수.
*/
void redirection(char *argv[])
{
    int file_descriptor; /*open() 리턴값을 저장할 변수.*/

    /*
    * argv[i]가 NULL일때까지 탐색.
    */
    for (int i = 0; argv[i] != NULL; i++) 
    {
        if (strcmp(argv[i], "<") == 0)
        {
            /*
            * argv[i+1], 즉 < 다음에 오는 파일을 읽기 전용으로 연다,
            * 읽기전용으로 연 파일을 표준입력으로 대체.
            */
            file_descriptor = open(argv[i+1], O_RDONLY);
            int validate = dup2(file_descriptor, STDIN_FILENO);
            
            /*
            * dup2함수 에러처리.
            */
            if (validate == -1)
            {
                printf("cannot open %s for input\n", argv[i+1]);
                fflush(stdout);
                exit(EXIT_FAILURE);
            }
        
            close(file_descriptor); /*열린 파일 닫기.*/
            argv[i] = NULL;/*execvp() 함수 실행을 위해 마지막 인덱스 값에 NULL넣기.*/
       
        }
        else if (strcmp(argv[i], ">") == 0)
        {
            /*
            * 파일을 읽기/쓰기 모드로 열고,
            * 열린 파일을 표준출력으로 대체.
            */
            file_descriptor = open(argv[i+1], O_CREAT | O_TRUNC | O_RDWR, 0744);
            int validate = dup2(file_descriptor, STDOUT_FILENO);
       
            if (validate == -1)
            {
                printf("cannot open %s for output\n", argv[i+1]);
                fflush(stdout);
                exit(EXIT_FAILURE);
            }
            close(file_descriptor);
            argv[i] = NULL;
            
          
        }
        
    }
    /*
    * execvp() 함수를 실행하고 에러처리.
    */
    if(execvp(argv[0], argv)<0)
    {
        perror("cmd error");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }
}

/*
* 명령어를 파싱하기 위한 함수.
*/
void parsing_cmd(char *cmd, char *argv[])
{
    int argc = 0;                 /* 인자의 개수 */
    char *p, *q;                  /* 명령어를 파싱하기 위한 변수 */
    /*
     * 명령어 앞부분 공백문자를 제거하고 인자를 하나씩 꺼내서 argv에 차례로 저장한다.
     * 작은 따옴표나 큰 따옴표로 이루어진 문자열을 하나의 인자로 처리한다.
     */
    p = cmd;
    p += strspn(p, " \t");
    do
    {
        /*
         * 공백문자, 큰 따옴표, 작은 따옴표가 있는지 검사한다.
         */
        q = strpbrk(p, " \t\'\"");
        /*
         * 공백문자가 있거나 아무 것도 없으면 공백문자까지 또는 전체를 하나의 인자로 처리한다.
         */
        if (q == NULL || *q == ' ' || *q == '\t')
        {
            q = strsep(&p, " \t");
            if (*q)
                argv[argc++] = q;
        }
        /*
         * 작은 따옴표가 있으면 그 위치까지 하나의 인자로 처리하고,
         * 작은 따옴표 위치에서 두 번째 작은 따옴표 위치까지 다음 인자로 처리한다.
         * 두 번째 작은 따옴표가 없으면 나머지 전체를 인자로 처리한다.
         */
        else if (*q == '\'')
        {
            q = strsep(&p, "\'");
            if (*q)
                argv[argc++] = q;
            q = strsep(&p, "\'");
            if (*q)
                argv[argc++] = q;
        }
        /*
         * 큰 따옴표가 있으면 그 위치까지 하나의 인자로 처리하고,
         * 큰 따옴표 위치에서 두 번째 큰 따옴표 위치까지 다음 인자로 처리한다.
         * 두 번째 큰 따옴표가 없으면 나머지 전체를 인자로 처리한다.
         */
        else
        {
            q = strsep(&p, "\"");
            if (*q)
                argv[argc++] = q;
            q = strsep(&p, "\"");
            if (*q)
                argv[argc++] = q;
        }

    } while (p);

    argv[argc] = NULL;
}

/*
* pipe기능을 구현한 함수이며, 재귀적으로 동작한다.
*/
void do_pipe(char *argv[])
{
    int fd[2]; /*pipe를 위한 파일 디스크립터 배열.*/
    pid_t pid; /*프로세스 id*/
    int pipe_idx = 0; /*pipe 위치를 가리키는 변수*/
    
    /*
    *pipe, 즉 "|"가 있는지 검사하고 pipe 위치를 파악한다.
    */
    for (int i = 0; argv[i] != NULL; i++)
    {
        if (strcmp(argv[i], "|") == 0)
        {
            pipe_idx = i;
            argv[i] = NULL;
            break;
        }
    }
    /*
    *"|"가 없는 경우에는 리다이렉션과 명렬 실행을 바로 수행하며,
    * 재귀함수의 종료조건이 된다.
    */
    if (pipe_idx == 0) {
        redirection(argv);
        // 리다이렉션 + 실행함수
    }
    /*
    * pip를 위한 파일 디스크립터를 생성하고,
    * 생성 실패 시 오류 메세지를 출력한다.
    */
    if (pipe(fd) == -1) {
        perror("pipe failed");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }
    
    /*
    * 손자 프로세스를 생성하고,
    * 생성 실패시 오류 메세지를 출력한다.
    */
    if((pid = fork()) == -1)
    {
        perror("fork failed");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }
    /*
    * 손자 프로세스인 경우.
    */
    if (pid == 0) {
        close(fd[READ_END]);/* pipe의 읽기용 파일 디스크립터를 닫는다.*/
        /*
        * 표준 출력을 pipe 쓰기용 파일 디스크립터로 연결하고,
        * 연결 실패 시 오류 메세지를 출력한다.
        */
        if(dup2(fd[WRITE_END], STDOUT_FILENO)<0)
        {
            perror("child process fail");
            fflush(stdout);
            exit(EXIT_FAILURE);
        }
        close(fd[WRITE_END]); /* pipe의 쓰기용 파일 디스크립터를 닫는다.*/
        redirection(argv);
       
    }
    /*자손 프로세스인 경우.*/
    close(fd[WRITE_END]);/* pipe의 쓰기용 파일 디스크립터를 닫는다.*/
    /*
    * 표준 입력을 pipe 읽기용 파일 디스크립터로 복제하고,
    * 복제 실패 시 오류 메세지를 출력한다.
    */
    if(dup2(fd[READ_END], STDIN_FILENO)<0)
    {
        perror("parent process fail");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }
    close(fd[READ_END]); /* 파이프의 읽기용 파일 디스크립터를 닫는다.*/
    do_pipe(&argv[pipe_idx + 1]); /* 재귀적으로 do_pipe를 호출하여 pipe 뒤쪽의 명령어를 실행한다.*/
}

/*
 * cmdexec - 명령어를 파싱해서 실행한다.
 * 스페이스와 탭을 공백문자로 간주하고, 연속된 공백문자는 하나의 공백문자로 축소한다.
 * 작은 따옴표나 큰 따옴표로 이루어진 문자열을 하나의 인자로 처리한다.
 * 기호 '<' 또는 '>'를 사용하여 표준 입출력을 파일로 바꾸거나,
 * 기호 '|'를 사용하여 파이프 명령을 실행하는 것도 여기에서 처리한다.
 */
static void cmdexec(char *cmd)
{
    char *argv[MAX_LINE / 2 + 1]; /* 명령어 인자를 저장하기 위한 배열 */
    
    parsing_cmd(cmd, argv); /* 명령어를 파싱하여 명령인자 배열 생성*/
    do_pipe(argv); /*파싱된 명령어를 실행*/
    
}

/*
 * 기능이 간단한 유닉스 셸인 tsh (tiny shell)의 메인 함수이다.
 * tsh은 프로세스 생성과 파이프를 통한 프로세스간 통신을 학습하기 위한 것으로
 * 백그라운드 실행, 파이프 명령, 표준 입출력 리다이렉션 일부만 지원한다.
 */
int main(void)
{
    char cmd[MAX_LINE + 1]; /* 명령어를 저장하기 위한 버퍼 */
    int len;                /* 입력된 명령어의 길이 */
    pid_t pid;              /* 자식 프로세스 아이디 */
    int background;         /* 백그라운드 실행 유무 */

    /*
     * 종료 명령인 "exit"이 입력될 때까지 루프를 무한 반복한다.
     */
    while (true)
    {
        /*
         * 좀비 (자식)프로세스가 있으면 제거한다.
         */
        pid = waitpid(-1, NULL, WNOHANG);
        if (pid > 0)
            printf("[%d] + done\n", pid);
        /*
         * 셸 프롬프트를 출력한다. 지연 출력을 방지하기 위해 출력버퍼를 강제로 비운다.
         */
        printf("tsh> ");
        fflush(stdout);

        /*
         * 표준 입력장치로부터 최대 MAX_LINE까지 명령어를 입력 받는다.
         * 입력된 명령어 끝에 있는 새줄문자를 널문자로 바꿔 C 문자열로 만든다.
         * 입력된 값이 없으면 새 명령어를 받기 위해 루프의 처음으로 간다.
         */
         
        len = read(STDIN_FILENO, cmd, MAX_LINE);
        if (len == -1)
        {
            perror("read");
            exit(EXIT_FAILURE);
        }
        cmd[--len] = '\0';
        if (len == 0)
            continue;
        /*
         * 종료 명령이면 루프를 빠져나간다.
         */
        if (!strcasecmp(cmd, "exit"))
            break;
        /*
         * 백그라운드 명령인지 확인하고, '&' 기호를 삭제한다.
         */
        char *p = strchr(cmd, '&');
        if (p != NULL)
        {
            background = 1;
            *p = '\0';
        }
        else
            background = 0;
        /*
         * 자식 프로세스를 생성하여 입력된 명령어를 실행하게 한다.
         */
        if ((pid = fork()) == -1)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        /*
         * 자식 프로세스는 명령어를 실행하고 종료한다.
         */
        else if (pid == 0)
        {
            cmdexec(cmd);
            exit(EXIT_SUCCESS);
        }
        /*
         * 포그라운드 실행이면 부모 프로세스는 자식이 끝날 때까지 기다린다.
         * 백그라운드 실행이면 기다리지 않고 다음 명령어를 입력받기 위해 루프의 처음으로 간다.
         */
        else if (!background)
            waitpid(pid, NULL, 0);
    }
    return 0;
}
