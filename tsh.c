/*
 * Copyright(c) 2023 All rights reserved by Heekuck Oh.
 * 이 프로그램은 한양대학교 ERICA 컴퓨터학부 학생을 위한 교육용으로 제작되었다.
 * 한양대학교 ERICA 학생이 아닌 이는 프로그램을 수정하거나 배포할 수 없다.
 * 프로그램을 수정할 경우 날짜, 학과, 학번, 이름, 수정 내용을 기록한다.
 */

/* 날짜: 2023.03.21
 * 학과: 컴퓨터학부
 * 학번: 2019092651
 * 이름: 이의형
 * 수정 내용: 표준 입출력 리다이렉션용 함수인 openfile()를 작성했다.
 * 인자로 argv 배열을 받아 < 와 > 를 처리해 준다.
 */

/* 날짜: 2023.03.22
 * 학과: 컴퓨터학부
 * 학번: 2019092651
 * 이름: 이의형
 * 수정 내용: pipe 기능을 하는 함수 makepipe()의 기초 틀을 작성했다.
 */

/* 날짜: 2023.03.22
 * 학과: 컴퓨터학부
 * 학번: 2019092651
 * 이름: 이의형
 * 수정 내용: 기존 cmdexec() 함수를 문자열을 파싱하는 함수로 변경했다.
 * main 함수에서 명령어를 받는 함수를 cmdexec() 함수에서 makepipe() 함수로 변경했다.*/

/* 날짜: 2023.03.26
 * 학과: 컴퓨터학부
 * 학번: 2019092651
 * 이름: 이의형
 * 수정 내용: makepipe() 함수가 기존의 의도대로 작동하도록 완성하였다.
 * 기존 의도는 cmdexec() 함수에서 파싱한 배열에서 for문을 돌며 '|'기호를 찾으면 우선 자식 프로세스를 생성한다.
 * 생성된 자식 프로세스에서 openfile() 함수로 '|'기호 앞의 명령어를 실행시킨다.*/

/* 날짜: 2023.03.29
 * 학과: 컴퓨터학부
 * 학번: 2019092651
 * 이름: 이의형
 * 수정 내용: 함수의 이름을 수정된 기능에 맞게 수정하고 기능을 설명하는 주석을 추가 및 수정하였다.
 * cmdexec() -> parse_cmd_argv(), openfile() -> execute_cmd(), makepipe() -> parse_pipe_and_fork()
 * 각 함수가 처리하는 부분을 함수로 쪼개서 가독성과 확장성을 챙겼다. 새로 추가된 함수에는
 * handle_input_redirection(), handle_output_redirection(), create_pipe(), create_child_process()
 * 총 4개가 된다.
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
#define READ_END 0  /* pipe 통로 중 읽기 전용 파일을 위한 정수값 */
#define WRITE_END 1 /* pipe 통로 중 쓰기 전용 파일을 위한 정수값 */
#define BUFFER_SIZE 25

static void parse_cmd_argv(char *cmd, char *argv[]);
static void execute_cmd(char *argv[]);
static void parse_pipe_and_fork(char *cmd);
static void handle_input_redirection(int *fd, char *filename[]);
static void handle_output_redirection(int *fd, char *filename[]);
static void create_pipe(int *fd);
static void create_child_process(pid_t *pid_ptr);

/*
 * parse_cmd_argv - 명령어를 파싱한다.
 * 매개변수 cmd를 받아 파싱 후 argv로 반환한다. 리턴값은 없다.
 * 스페이스와 탭을 공백문자로 간주하고, 연속된 공백문자는 하나의 공백문자로 축소한다.
 * 작은 따옴표나 큰 따옴표로 이루어진 문자열을 하나의 인자로 처리한다.
 */
static void parse_cmd_argv(char *cmd, char *argv[])
{
    int argc = 0; /* 인자의 개수 */
    char *p, *q;  /* 명령어를 파싱하기 위한 변수 */

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
         *   strpbrk() 함수는 문자에 대한 포인터를 리턴한다.
         *   string1 및 string2에 공통된 문자가 없으면 NULL 포인터가 리턴된다.
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

    /* 배열의 마지막에 NULL을 넣어 exec 계열 함수가 끝을 알 수 있게 한다. */
    argv[argc] = NULL;
}

/*
 * execute_cmd - 명령어를 실행한다.
 * 기호인 '<', '>' 가 있으면 handle_input_redirection()
 * 또는 handle_output_redirection() 함수를 호출해서 리다이렉션을 처리한다.
 */
static void execute_cmd(char *argv[])
{
    int fd; /* 파일 디스크립트 */
    
    for (int i = 0; argv[i] != NULL; i++)
    {
        /*
         * 기호 "<"를 만났을 때 함수 handle_input_redirection()을 실행한다.
         * 배열에서 "<"를 파라미터의 끝을 의미하는 NULL로 치환한다.
         */
        if (strcmp(argv[i], "<") == 0)
        {
            handle_input_redirection(&fd, &argv[i + 1]);
            argv[i] = NULL; /* 기호가 있던 자리에 NULL 값으로 치환한다. */
        }
        /*
         * 기호 ">"를 만났을 때 함수 handle_output_redirection()을 실행한다.
         * 배열에서 ">"를 파라미터의 끝을 의미하는 NULL로 치환한다.
         */
        else if (strcmp(argv[i], ">") == 0)
        {
            handle_output_redirection(&fd, &argv[i + 1]);
            argv[i] = NULL; /* 기호가 있던 자리에 NULL 값으로 치환한다. */
        }
    }

    /*
     * 배열 argv에서 첫번째 인덱스의 값은 명령어다.
     * excvp() 함수는 첫 파라미터로 str 타입의 명령어를 받고, 두 번째 파라미터로 배열을 받는다.
     * 정상적으로 함수가 처리가 되면 리턴값은 없다.
     * 만약 함수 실행에 오류가 난다면 -1 값을 리턴한다. 오류 메세지로 "execvp"를 출력한다.
     */
    if (execvp(argv[0], argv) == -1)
    {
        perror("execvp");
        exit(EXIT_FAILURE);
    }
}

/*
 * handle_input_redirection - 주어진 파일명(filename)으로 입력 부분을 리다이렉션한다.
 * open() 함수로 주어진 파일을 읽기 전용으로 열어둔다.
 * dup2() 함수로 더이상 표준 입력이 아닌 파일에서 읽어오게 한다.
 * close() 함수로 파일을 닫는다 */
static void handle_input_redirection(int *fd, char *filename[])
{
    *fd = open(*filename, O_RDONLY); /* 파일 열기 */

    /*
     * 파일 열기에 실패했을 때 오류 메세지로 "open"을 출력한다.
     */
    if (*fd < 0)
    {
        perror("open");
        exit(1);
    }
    dup2(*fd, STDIN_FILENO); /* 입력을 키보드가 아닌 파일에서 입력을 받는다. */
    close(*fd);              /* 파일을 닫습니다. */
}

/*
 * handle_output_redirection - 주어진 파일명(filename)으로 출력 부분을 리다이렉션한다.
 * open() 함수로 주어진 파일을 쓰기기 전용으로 열거나, 파일이 없으면 생성한다.
 * dup2() 함수로 더이상 표준 출력이 아닌 파일에서 출력하게 한다.
 * close() 함수로 파일을 닫는다*/
static void handle_output_redirection(int *fd, char *filename[])
{
    *fd = open(*filename, O_WRONLY | O_CREAT, 0644); /* 파일 쓰기 또는 생성 */

    /*
     * 파일 열기에 실패했을 때 오류 메세지로 "open"을 출력한다.
     */
    if (*fd < 0)
    {
        perror("open");
        exit(1);
    }
    dup2(*fd, STDOUT_FILENO); /* 출력을 화면이 아닌 파일에 출력을 한다. */
    close(*fd);               /* 파일을 닫는다. */
}

/*
 * parse_pipe_and_fork - 파이프 기호를 찾고 자식 프로세스를 생성한다.
 * 매개변수 cmd를 parse_cmd_argv()함수에서 파싱한 배열이 argv다.
 * argv 배열에서 '|'기호를 찾으면 create_pipe() 함수를 호출하여 파이프를 생성한다.
 * 이후 creat_child_process() 함수로 자식 프로세스를 생성한 뒤 자식 프로세스가 명령어를 처리하게 한다.
 */
static void parse_pipe_and_fork(char *cmd)
{
    char *argv[MAX_LINE / 2 + 1]; /* 명령어 인자를 저장하기 위한 배열 */
    parse_cmd_argv(cmd, argv);    /* cmd 명령어를 파싱하고 argv로 반환받는다. */
    int fd[2];                    /* 파일 디스크립트 배열 */
    pid_t pid;                    /* 프로세스 아이디 */
    int pipe_start_index = 0;     /* 파이프 기준으로 명령어를 나누기 위한 시작점 인덱스 */

    /*
     *'|' 기호를 처리하기 위한 for문 이다.
     */
    for (int i = 0; argv[i] != NULL; i++)
    {
        if (*argv[i] == '|') /* 파이프 기호를 만났을 때 실행된다. */
        {
            argv[i] = NULL;
            /*
             * 자식 프로세스를 만들기 전(fork() 하기 전)에 먼저 파이프를 생성한다.
             */
            create_pipe(fd);
            /*
             * 자식 프로세스를 생성한다.
             */
            create_child_process(&pid);
            /*
             * 자식 프로세스는 명령어를 실행한다.
             */
            if (pid == 0)
            {
                close(fd[READ_END]); /* 자식 프로세스의 부모로부터 읽는 기능은 닫는다. */
                /*
                 * 파이프의 쓰기용 디스크립트를 표준 출력에 복사를 한다.
                 * 이후 출력하는 작업은 모두 파이프의 쓰기용 디스크립트와 동일하게 작용한다.
                 */
                dup2(fd[WRITE_END], STDOUT_FILENO);
                close(fd[WRITE_END]); /* 사용 후 파일 디스크립트는 닫는다. */

                execute_cmd(&argv[pipe_start_index]); /* 받은 배열의 명령을 실행한다. */   
            }
            /*
             * 부모 프로세스
             */
            else
            {
                close(fd[WRITE_END]); /* 부모 프로세스의 자식에게 쓰는 기능은 닫습니다. */
                /*
                 * 파이프의 읽기용 디스크립트를 표준 입력에 복사를 한다.
                 * 이후 입력하는 작업은 모두 파이프의 읽기용 디스크립트와 동일하게 작용한다.
                 */
                dup2(fd[READ_END], STDIN_FILENO);

                close(fd[READ_END]); /* 사용 후 파일 디스크립트는 닫는다. */
            }
            pipe_start_index = i + 1; /* 다음 명령어의 시작 인덱스를 가리킨다. */
        }
    }

    /* 
     * 남은 명령어 배열을 처리한다. 
     */
    execute_cmd(&argv[pipe_start_index]); 
}

/*
 * create_pipe - 주어진 파일 디스크립트 배열로 파이프를 생성하는 함수
 * 생성에 실패하면 오류를 출력한다.
 */
static void create_pipe(int *fd)
{
    /* 
     * pipe를 생성하며 에러가 나면 if문이 실행된다. 
     */
    if (pipe(fd) == -1) 
    {
        fprintf(stderr, "Pipe failed");
        exit(EXIT_FAILURE);
    }
}

/*
 * create_child_process - fork()를 하여 프로세스를 생성한다.
 * 생성에 실패하면 오류를 출력한다.
 */
static void create_child_process(pid_t *pid_ptr)
{
    pid_t pid = fork(); /* 자식 프로세스를 생성한다. */

    /* 
     *fork에 실패하였을때 호출되어 "Fork Failed"를 출력한다.
     */
    if (pid < 0)        
    {
        fprintf(stderr, "Fork Failed");
        exit(EXIT_FAILURE);
    }

    *pid_ptr = pid; /* 주어진 인자에 넣는다. */
}

/*
 * main - 기능이 간단한 유닉스 셸인 tsh (tiny shell)의 메인 함수이다.
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
        { // -1 값은 오류로 종료한다
            perror("read");
            exit(EXIT_FAILURE);
        }
        cmd[--len] = '\0';
        if (len == 0)
            continue;
        /*
         * 종료 명령이면 루프를 빠져나간다.
         * strcasecmp는 대소문자를 구분하지 않고 두 문자를 비교한다.
         * 같으면 0을, 다르면 0이 아닌 값을 리턴한다.
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
            parse_pipe_and_fork(cmd);
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
