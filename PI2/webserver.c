#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <wiringPi.h>

#include <curl/curl.h>

typedef enum _TOKEN_TYPE{
	TOKEN_STRING,
	TOKEN_NUMBER,
} TOKEN_TYPE;

typedef struct _TOKEN{
	TOKEN_TYPE type;
	union{
		char *string;
		double number;
	};
	bool isArray;
}TOKEN;

#define TOKEN_COUNT 20

typedef struct _JSON{
	TOKEN tokens[TOKEN_COUNT];
} JSON;

/* 쓰레드 처리를 위한 함수 */
static void *clnt_connection(void *arg);
int sendData(int fd, FILE* fp, char *ct, char *file_name);
void sendOk(FILE* fp);
void sendError(FILE* fp);

void parseJSON(char *doc, int size, JSON *json);
char *readFile(char *filename, int *readSize);
int jsonStart();
void freeJSON(JSON *json);

static size_t write_data(void *ptr, size_t size, size_t nmeb, void *stream);
void reciveToServer();

int main(int argc, char **argv)
{
  int serv_sock, i;
  pthread_t thread;
  struct sockaddr_in serv_addr, clnt_addr;
  unsigned int clnt_addr_size;

  if(wiringPiSetup() == -1){
	printf("wiringPi error!\n");
	return -1;
  }

  if(argc!=2) {
    printf("usage: %s <port>\n", argv[0]);
    return -1; 
  }
   
  /* 서버를 위한 소켓을 생성한다. */
  serv_sock = socket(PF_INET, SOCK_STREAM, 0);
  if(serv_sock == -1) {
    perror("socket( )");
    return -1; 
  }

  /* 입력받는 포트번호를 이용해서 서비스를 운영체제에 등록한다. */
  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = (argc != 2)?htons(8000):htons(atoi(argv[1]));
  if(bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr))==-1) {
    perror("bind( )");
    return -1; 
  }

  /* 최대 10대의 클라이언트를 처리할 수 있도록 큐를 생성한다. */
  if(listen(serv_sock, 10) == -1) {
    perror("listen( )");
    return -1; 
  }

  while(1) {
    int clnt_sock;
    /* 클라이언트의 요청을 기다린다. */
    clnt_addr_size = sizeof(clnt_addr);
    clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
    printf("Client IP : %s:%d\n", inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));

    /* 클라이언트의 요청이 들어오면 쓰레드를 생성하고 클라이언트의 요청을 처리한다. */
    pthread_create(&thread,NULL, clnt_connection, &clnt_sock);
    pthread_join(thread, 0);
  };

  return 0;
}

void *clnt_connection(void *arg)
{
  /* 쓰레드를 통해서 넘어온 arg를 int 형의 파일 디스크립터로 변환한다. */
  int clnt_sock = *((int*)arg), clnt_fd;
  FILE *clnt_read, *clnt_write;
  char reg_line[BUFSIZ], reg_buf[BUFSIZ];
  char method[10], ct[BUFSIZ], type[BUFSIZ];
  char file_name[256], file_buf[256];
  char* type_buf;
  int i = 0, j = 0, len = 0;

  /* 파일 디스크립터를 FILE 스트림으로 변환한다. */
  clnt_read = fdopen(clnt_sock, "r");
  clnt_write = fdopen(dup(clnt_sock), "w");
  clnt_fd = clnt_sock;

  /* 한 줄의 문자열을 읽어서 reg_line 변수에 저장한다. */
  fgets(reg_line, BUFSIZ, clnt_read);
  /* reg_line 변수에 문자열을 화면에 출력한다. */
  fputs(reg_line, stdout);

  /* '/' 문자로 reg_line을 구분해서 요청 라인의 내용(메소드)를 분리한다. */
  strcpy(method, strtok(reg_line, " "));
  if(strcmp(method, "POST") == 0) {                  /* POST메소드일 경우를 처리한다. */
    sendOk(clnt_write);                                   /* 단순히 OK 메시지를 클라이언트로 보낸다. */
    fclose(clnt_read);
    fclose(clnt_write);
    return (void*)NULL;
  } else if(strcmp(method, "GET") != 0) {              /* GET 메소드가 아닐 경우를 처리한다. */
    sendError(clnt_write);                                      /* 에러 메시지를 클라이언트로 보낸다. */
    fclose(clnt_read);
    fclose(clnt_write);
    return (void*)NULL;
  }

  strcpy(file_name, strtok(NULL, " "));                /* 요청 라인에서 경로(path)를 가져온다. */
  if(file_name[0] == '/') {                                        /* 경로가 '/'로 시작될 경우 /를 제거한다. */
    for(i = 0, j = 0; i < BUFSIZ; i++) {
      if(file_name[0] == '/') j++;
      file_name[i] = file_name[j++];
      if(file_name[i+1] == '\0') break;
    };
  }

    /* 라즈베리 파이를 제어하기 위한 HTML 코드를 분석해서 처리한다. */
    if(strstr(file_name, "?") != NULL) {
        char optLine[32];
        char optStr[4][16];
        char opt[8], var[8];
        char* tok;
        int i, j, count = 0;
		char recipe_id[8], sign[8];

        strcpy(file_name, strtok(file_name, "?"));
        strcpy(optLine, strtok(NULL, "?"));

        /* 옵션을 분석한다. */
        tok = strtok(optLine, "&");
        while(tok != NULL) {
            strcpy(optStr[count++], tok);
            tok = strtok(NULL, "&");
        };

        /* 분석한 옵션을 처리한다. */
        for(i = 0; i < count; i++) {
            strcpy(opt, strtok(optStr[i], "="));
            strcpy(var, strtok(NULL, "="));
	   		printf("if ->~~~~~~~~~~~~~~~~~ %s = %s\n", opt, var);
			if(!strcmp(opt, "recipe_id")){
				strcpy(recipe_id, var);
			}else if(!strcmp(opt, "sign")){
				strcpy(sign, var);
			}
        };
		if(!strcmp(var, "ok")){
			reciveToServer();
			jsonStart();

		}
    }

  /* 요청 헤더를 읽어서 화면에 출력하고 나머지는 무시한다. */
  do {
    fgets(reg_line, BUFSIZ, clnt_read);
    fputs(reg_line, stdout);
    strcpy(reg_buf, reg_line);
    type_buf = strchr(reg_buf, ':');
  } while(strncmp(reg_line, "\r\n", 2));              /* 요청헤더의 마지막은 '\r\n\r\n'으로 끝난다. */

  /* 파일의 이름을 이용해서 클라이언트로 파일의 내용을 보낸다. */
  strcpy(file_buf, file_name);
  sendData(clnt_fd, clnt_write, ct, file_name);

  fclose(clnt_read);                                               /* 파일의 스트림을 닫는다. */
  fclose(clnt_write);

  pthread_exit(0);                                               /* 쓰레드를 종료시킨다. */

  return (void*)NULL;
}
 
int sendData(int fd, FILE* fp, char *ct, char *file_name)
{
  /* 클라이언트로 보낼 성공에 대한 응답 메시지 */
  char protocol[] = "HTTP/1.1 200 OK\r\n";
  char server[] = "Server:Netscape-Enterprise/6.0\r\n";
  char cnt_type[] = "Content-Type:text/html\r\n";
  char end[] = "\r\n";                                      /* 헤더의 끝은 항상 \r\n */
  char buf[BUFSIZ];
  int len;

  fputs(protocol, fp);
  fputs(server, fp);
  fputs(cnt_type, fp);
  fputs(end, fp);
  fflush(fp);

  fd = open(file_name, O_RDWR);               /* 파일을 오픈한다. */

  do {
    len = read(fd, buf, BUFSIZ);                     /* 파일을 읽어서 클라이언트로 보낸다. */
    fwrite(buf, len, sizeof(char), fp);
  } while(len == BUFSIZ);

  fflush(fp);
  close(fd);                                                     /* 파일을 닫는다. */

  return 0;
}

void sendOk(FILE* fp)
{
  /* 클라이언트에 보낼 성공에 대한 HTTP 응답 메시지 */
  char protocol[] = "HTTP/1.1 200 OK\r\n";
  char server[] = "Server: Netscape-Enterprise/6.0\r\n\r\n";

  fputs(protocol, fp);
  fputs(server, fp);
  fflush(fp); 
}

void sendError(FILE* fp)
{
  /* 클라이언트로 보낼 실패에 대한 HTTP 응답 메시지 */
  char protocol[] = "HTTP/1.1 400 Bad Request\r\n";
  char server[] = "Server: Netscape-Enterprise/6.0\r\n";
  char cnt_len[] = "Content-Length:1024\r\n";
  char cnt_type[] = "Content-Type:text/html\r\n\r\n";

  /* 화면에 표시될 HTML의 내용 */
  char content1[] = "<html><head><title>BAD Connection</tiitle></head>";
  char content2[] = "<body><font size=+5>Bad Request</font></body></html>";

  printf("send_error\n");
  fputs(protocol, fp);
  fputs(server, fp);
  fputs(cnt_len, fp);
  fputs(cnt_type, fp);
  fputs(content1, fp);
  fputs(content2, fp);
  fflush(fp); 
}

////////////////////////////////// start reciveToSever////////////////////////
static size_t write_data(void *ptr, size_t size, size_t nmeb, void *stream){
	size_t written = fwrite(ptr, size, nmeb, (FILE *)stream);
  	return written;
}

void reciveToServer(){
	CURL *curl_handle;
	char *urlpath = "http://192.168.100.100:8080/tutorial/api/recipe/data/1";
	static const char *pagefilename = "recipe.json";
	FILE *pagefile;
 
	curl_global_init(CURL_GLOBAL_ALL); 
	curl_handle = curl_easy_init();
	curl_easy_setopt(curl_handle, CURLOPT_URL, urlpath);
	curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);
	curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);

	pagefile = fopen(pagefilename, "wb");
	if(pagefile) {	
		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, pagefile);
		curl_easy_perform(curl_handle);
		fclose(pagefile);
	}
  
	curl_easy_cleanup(curl_handle);
 
}

/////////////////////////////// start Json//////////////////////////

char *readFile(char *filename, int *readSize){

	FILE *fp = fopen(filename, "rb");
	if(fp == NULL)
		return NULL;

	int size;
	char *buffer;

	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	buffer = malloc(size+1);
	memset(buffer, 0, size);

	if(fread(buffer, size, 1, fp) < 1){
		*readSize = 0;
		free(buffer);
		fclose(fp);
		return NULL;
	}
	*readSize = size;
	fclose(fp);
	return buffer;
}


void parseJSON(char *doc, int size, JSON *json){
	int tokenIndex = 0;
	int pos = 0;

	if(doc[pos] != '{')
		return;

	pos++;

	while(pos < size){
		switch(doc[pos]){
			case '"':
			{
				char *begin = doc + pos + 1;

				char *end = strchr(begin, '"');
				if(end == NULL)
					break;

				int stringLength = end - begin;

				json->tokens[tokenIndex].type = TOKEN_STRING;
				json->tokens[tokenIndex].string = malloc(stringLength + 1);
				memset(json->tokens[tokenIndex].string, 0, stringLength + 1);

				memcpy(json->tokens[tokenIndex].string, begin, stringLength);

				tokenIndex++;

				pos = pos + stringLength + 1;
			}
			break;
		}
		pos++;
	}
}

void freeJSON(JSON *json){
	int i;
	for(i=0 ; i<TOKEN_COUNT ; i++){
		if(json->tokens[i].type == TOKEN_STRING)
			free(json->tokens[i].string);
	}
}

int jsonStart(){
	int size;
	char *doc = readFile("recipe.json", &size);
	if( doc == NULL )
		return -1;

	JSON json= { 0, };
	parseJSON(doc, size, &json);

	printf("%s\n", json.tokens[0].string);
	printf("%s\n", json.tokens[1].string);
	printf("%s\n", json.tokens[2].string);
	printf("%s\n", json.tokens[3].string);
	printf("%s\n", json.tokens[4].string);
	printf("%s\n", json.tokens[5].string);
	printf("%s\n", json.tokens[6].string);
	printf("%s\n", json.tokens[7].string);
	printf("%s\n", json.tokens[8].string);
	
	freeJSON(&json);
	free(doc);

	return 0;	
}
