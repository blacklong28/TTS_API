/*
* 语音听写(iFly Auto Transform)技术能够实时地将语音转换成对应的文字。
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "qisr.h"
#include "msp_cmn.h"
#include "msp_errors.h"
#include "speech_recognizer.h"
#include "cJSON.h"
#include "qtts.h"
#include <alsa/asoundlib.h>
#include"asr_tts_awake.h"
/*
* 语音语义技术能够将语音听写业务中的内容进行语义解析。
*/


#define SOURCETEXT  "source.txt" //语义文本资源
#define RESULTTEXT  "result.txt" //语义结果文本


#define HINTS_SIZE  100


#define FRAME_LEN	640 
#define	BUFFER_SIZE	4096



 struct WAV_HEADER
{
    char rld[4]; //riff 标志符号
    int rLen;
    char wld[4]; //格式类型（wave）
    char fld[4]; //"fmt"

    int fLen; //sizeof(wave format matex)
    
    short wFormatTag; //编码格式
    short wChannels; //声道数
    int nSamplesPersec ; //采样频率
    int nAvgBitsPerSample;//WAVE文件采样大小
    short wBlockAlign; //块对齐
    short wBitsPerSample; //WAVE文件采样大小
    
    char dld[4]; //”data“
    int wSampleLength; //音频数据的大小

} wav_header;


/* Upload User words */
static int upload_userwords()
{
	char*			userwords	=	NULL;
	size_t			len			=	0;
	size_t			read_len	=	0;
	FILE*			fp			=	NULL;
	int				ret			=	-1;

	fp = fopen("userwords.txt", "rb");
	if (NULL == fp)										
	{
		printf("\nopen [userwords.txt] failed! \n");
		goto upload_exit;
	}

	fseek(fp, 0, SEEK_END);
	len = ftell(fp); 
	fseek(fp, 0, SEEK_SET);  					
	
	userwords = (char*)malloc(len + 1);
	if (NULL == userwords)
	{
		printf("\nout of memory! \n");
		goto upload_exit;
	}

	read_len = fread((void*)userwords, 1, len, fp); 
	if (read_len != len)
	{
		printf("\nread [userwords.txt] failed!\n");
		goto upload_exit;
	}
	userwords[len] = '\0';
	
	MSPUploadData("userwords", userwords, len, "sub = uup, dtt = userword", &ret); //ÉÏ´«ÓÃ»§´Ê±í
	if (MSP_SUCCESS != ret)
	{
		printf("\nMSPUploadData failed ! errorCode: %d \n", ret);
		goto upload_exit;
	}
	
upload_exit:
	if (NULL != fp)
	{
		fclose(fp);
		fp = NULL;
	}	
	if (NULL != userwords)
	{
		free(userwords);
		userwords = NULL;
	}
	
	return ret;
}


static void show_result(char *string, char is_over)
{
	printf("\rResult: [ %s ]", string);
	if(is_over)
		putchar('\n');
}

static char *g_result = NULL;
static unsigned int g_buffersize = BUFFER_SIZE;


/* demo send audio data from a file */
static void demo_file(const char* audio_file, const char* session_begin_params)
{
	int	errcode = 0;
	FILE*	f_pcm = NULL;
	char*	p_pcm = NULL;
	unsigned long	pcm_count = 0;
	unsigned long	pcm_size = 0;
	unsigned long	read_size = 0;
	struct speech_rec iat;
	struct speech_rec_notifier recnotifier = {
		on_result,
		on_speech_begin,
		on_speech_end
	};

	if (NULL == audio_file)
		goto iat_exit;

	f_pcm = fopen(audio_file, "rb");
	if (NULL == f_pcm)
	{
		printf("\nopen [%s] failed! \n", audio_file);
		goto iat_exit;
	}

	fseek(f_pcm, 0, SEEK_END);
	pcm_size = ftell(f_pcm);
	fseek(f_pcm, 0, SEEK_SET);

	p_pcm = (char *)malloc(pcm_size);
	if (NULL == p_pcm)
	{
		printf("\nout of memory! \n");
		goto iat_exit;
	}

	read_size = fread((void *)p_pcm, 1, pcm_size, f_pcm);
	if (read_size != pcm_size)
	{
		printf("\nread [%s] error!\n", audio_file);
		goto iat_exit;
	}

	errcode = sr_init(&iat, session_begin_params, SR_USER, &recnotifier);
	if (errcode) {
		printf("speech recognizer init failed : %d\n", errcode);
		goto iat_exit;
	}

	errcode = sr_start_listening(&iat);
	if (errcode) {
		printf("\nsr_start_listening failed! error code:%d\n", errcode);
		goto iat_exit;
	}

	while (1)
	{
		unsigned int len = 10 * FRAME_LEN; /* 200ms audio */
		int ret = 0;

		if (pcm_size < 2 * len)
			len = pcm_size;
		if (len <= 0)
			break;

		ret = sr_write_audio_data(&iat, &p_pcm[pcm_count], len);

		if (0 != ret)
		{
			printf("\nwrite audio data failed! error code:%d\n", ret);
			goto iat_exit;
		}

		pcm_count += (long)len;
		pcm_size -= (long)len;		
	}

	errcode = sr_stop_listening(&iat);
	if (errcode) {
		printf("\nsr_stop_listening failed! error code:%d \n", errcode);
		goto iat_exit;
	}

iat_exit:
	if (NULL != f_pcm)
	{
		fclose(f_pcm);
		f_pcm = NULL;
	}
	if (NULL != p_pcm)
	{
		free(p_pcm);
		p_pcm = NULL;
	}

	sr_stop_listening(&iat);
	sr_uninit(&iat);
}

/* demo recognize the audio from microphone */
static void demo_mic(const char* session_begin_params,char*result)
{
	int errcode;
	int i = 0;

	struct speech_rec iat;

	struct speech_rec_notifier recnotifier = {
		on_result,
		on_speech_begin,
		on_speech_end,
		result
	};

	errcode = sr_init(&iat, session_begin_params, SR_MIC, &recnotifier);
	if (errcode) {
		printf("speech recognizer init failed\n");
		return;
	}
	errcode = sr_start_listening(&iat);
	if (errcode) {
		printf("start listen failed %d\n", errcode);
	}
	/* demo 15 seconds recording */
	while(i++ < 6)
		sleep(1);
	errcode = sr_stop_listening(&iat);
	if (errcode) {
		printf("stop listening failed %d\n", errcode);
	}

	sr_uninit(&iat);
}


/* main thread: start/stop record ; query the result of recgonization.
 * record thread: record callback(data write)
 * helper thread: ui(keystroke detection)
 */
int iat(char*text)
{
	int ret = MSP_SUCCESS;
	int upload_on =	1; /* whether upload the user word */
	/* login params, please do keep the appid correct */
	const char* login_params = "appid = 5982f5e2, proxy_ip=10.31.11.26,proxy_port=10000,work_dir = .";
	int aud_src = 0; /* from mic or file */

	/*
	* See "iFlytek MSC Reference Manual"
	*/
	const char* session_begin_params =
		"net_type=custom,sub = iat, domain = iat, language = zh_cn, "
		"accent = mandarin, sample_rate = 16000, "
		"result_type = plain, result_encoding = utf8";

	/* Login first. the 1st arg is username, the 2nd arg is password
	 * just set them as NULL. the 3rd arg is login paramertes 
	 * */
	ret = MSPLogin(NULL, NULL, login_params);
	if (MSP_SUCCESS != ret)	{
		printf("MSPLogin failed , Error code %d.\n",ret);
		goto exit; // login fail, exit the program
	}
	/*
	printf("Want to upload the user words ? \n0: No.\n1: Yes\n");
	scanf("%d", &upload_on);
	if (upload_on)
	{
		printf("Uploading the user words ...\n");
		ret = upload_userwords();
		if (MSP_SUCCESS != ret)
			goto exit;	
		printf("Uploaded successfully\n");
	}

	printf("Where the audio comes from?\n"
			"0: From a audio file.\n1: From microphone.\n");
	scanf("%d", &aud_src);
	if(aud_src != 0) {
		printf("Demo recognizing the speech from microphone\n");
		printf("Speak in 15 seconds\n");
	*/
		demo_mic(session_begin_params,text);
	/*
		printf("15 sec passed\n");
	} else {
		printf("Demo recgonizing the speech from a recorded audio file\n");
		demo_file("wav/iflytek02.wav", session_begin_params); 
	}
	*/
exit:
	MSPLogout(); // Logout...

	return 0;
}




int sch(char*iat_result,const char*tts_result)
{
	const char*  login_params	 =	"appid = 5982f5e2,proxy_ip=10.31.11.26,proxy_port=10000"; // 登录参数，appid与msc库绑定,请勿随意改动
	char*  rec_text        =   NULL;
	unsigned int str_len         =   0;
	int	         ret		     =	 MSP_SUCCESS;
	FILE*        fw              =   NULL;
	FILE*        fr              =   NULL;
	long         txtSize         =   0;
	long		 read_size	     =	 0;
	char*        text            =   NULL;
	/* 用户登录 */
	ret = MSPLogin(NULL, NULL, login_params); //第一个参数是用户名，第二个参数是密码，均传NULL即可，第三个参数是登录参数	
	if (MSP_SUCCESS != ret)
	{
		printf("MSPLogin failed , Error code %d.\n",ret);
		goto exit; //登录失败，退出登录
	}
	/*
	fr=fopen(SOURCETEXT,"rb");
	if(NULL == fr)
	{
		printf("\nopen [%s] failed! \n",SOURCETEXT);
		goto exit;
	}

	fseek(fr, 0, SEEK_END);
	txtSize = ftell(fr);
	fseek(fr, 0, SEEK_SET);

	text = (char *)malloc(txtSize+1);
	if (NULL == text)
	{
		printf("\nout of memory! \n");
		goto exit;
	}

	read_size = fread((void *)text,1, txtSize, fr);
	if (read_size != txtSize)
	{
		printf("\nread [%s] error!\n", SOURCETEXT);
		goto exit;
	}
	text[txtSize]='\0';
	*/
	str_len = strlen(iat_result);

	printf("\n开始语义解析...\n");
	rec_text = MSPSearch("net_type=custom,nlp_version=2.0",iat_result,&str_len,&ret);
	if(MSP_SUCCESS !=ret)
	{
		printf("MSPSearch failed ,error code is:%d\n",ret);
		goto exit;
	}
	printf("\n语义解析完成!\n");
	
	fw=fopen(RESULTTEXT,"wb");
	if(NULL == fw)
	{
		printf("\nopen [%s] failed! \n",RESULTTEXT);
		goto exit;
	}

	read_size = fwrite(rec_text,1,str_len,fw);
	if(read_size != str_len)
	{
		printf("\nwrite [%s] error!\n", RESULTTEXT);
		goto exit;
	}
	printf("\n语义解析结果已写入%s文件\n",RESULTTEXT);
	
	cJSON *json = cJSON_Parse(rec_text);
	if (json) {
		int rc = cJSON_GetObjectItem(json, "rc")->valueint;
		cJSON *answer = cJSON_GetObjectItem(json, "answer");
		cJSON *text_js = cJSON_GetObjectItem(answer, "text");
		strcat(tts_result,text_js->valuestring);
		//tts_result= text_js->valuestring;
	}
	//printf("tts_result1=%s",tts_result);
	//printf("tts_result2=%s",tts_result);
	//tts_result = rec_text;
	//tts(rec_text);
exit:
	if (NULL != fr)
	{
		fclose(fr);
		fr = NULL;
	}
	if (NULL != fw)
	{
		fclose(fw);
		fw = NULL;
	}
	if (NULL != text)
	{
		free(text);
		text = NULL;
	}
	//printf("\n按任意键退出 ...\n");
	//getchar();
	MSPLogout(); //退出登录

	return 0;
}


