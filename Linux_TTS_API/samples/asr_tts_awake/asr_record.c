/*
* 语音听写(iFly Auto Transform)技术能够实时地将语音转换成对应的文字。
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include"asr_tts_awake.h"
#include "cJSON.h"
#include "../../include/qisr.h"
#include "../../include/msp_cmn.h"
#include "../../include/msp_errors.h"
#include "speech_recognizer.h"


const char * ASR_RES_PATH        = "fo|res/asr/common.jet"; //离线语法识别资源路径
const char * GRM_BUILD_PATH      = "res/asr/GrmBuilld"; //构建离线语法识别网络生成数据保存路径
const char * GRM_FILE            = "call.bnf"; //构建离线识别语法网络所用的语法文件
const char * LEX_NAME            = "contact"; //更新离线识别语法的contact槽（语法文件为此示例中使用的call.bnf）



const char* get_audio_file(void)
{
	int key = 0;
	while(key != 27) //按Esc则退出
	{
		printf("请选择音频文件：\n");
		printf("1.打电话给丁伟\n");
		printf("2.打电话给黄辣椒\n");
		scanf("%d", &key);
		//key = getc();
		//printf("key==========%c",key);
		switch(key)
		{
		case 1:
			printf("\n1.打电话给丁伟\n");
			return "wav/ddhgdw.pcm";
		case 2:
			printf("\n2.打电话给黄辣椒\n");
			return "wav/ddhghlj.pcm";
		default:
			continue;
		}
	}
	exit(0);
	return NULL;
}

int build_grm_cb(int ecode, const char *info, void *udata)
{
	UserData *grm_data = (UserData *)udata;

	if (NULL != grm_data) {
		grm_data->build_fini = 1;
		grm_data->errcode = ecode;
	}

	if (MSP_SUCCESS == ecode && NULL != info) {
		printf("构建语法成功！ 语法ID:%s\n", info);
		if (NULL != grm_data)
			snprintf(grm_data->grammar_id, MAX_GRAMMARID_LEN - 1, info);
	}
	else
		printf("构建语法失败！%d\n", ecode);

	return 0;
}

int build_grammar(UserData *udata,char*grm_file_name)
{
	FILE *grm_file                           = NULL;
	char *grm_content                        = NULL;
	unsigned int grm_cnt_len                 = 0;
	char grm_build_params[MAX_PARAMS_LEN]    = {NULL};
	int ret                                  = 0;

	grm_file = fopen(grm_file_name, "rb");	
	if(NULL == grm_file) {
		printf("打开\"%s\"文件失败！[%s]\n", GRM_FILE, strerror(errno));
		return -1; 
	}

	fseek(grm_file, 0, SEEK_END);
	grm_cnt_len = ftell(grm_file);
	fseek(grm_file, 0, SEEK_SET);

	grm_content = (char *)malloc(grm_cnt_len + 1);
	if (NULL == grm_content)
	{
		printf("内存分配失败!\n");
		fclose(grm_file);
		grm_file = NULL;
		return -1;
	}
	fread((void*)grm_content, 1, grm_cnt_len, grm_file);
	grm_content[grm_cnt_len] = '\0';
	fclose(grm_file);
	grm_file = NULL;

	snprintf(grm_build_params, MAX_PARAMS_LEN - 1, 
		"engine_type = local, \
		asr_res_path = %s, sample_rate = %d, \
		grm_build_path = %s, ",
		ASR_RES_PATH,
		SAMPLE_RATE_16K,
		GRM_BUILD_PATH
		);
	ret = QISRBuildGrammar("bnf", grm_content, grm_cnt_len, grm_build_params, build_grm_cb, udata);

	free(grm_content);
	grm_content = NULL;

	return ret;
}

int update_lex_cb(int ecode, const char *info, void *udata)
{
	UserData *lex_data = (UserData *)udata;

	if (NULL != lex_data) {
		lex_data->update_fini = 1;
		lex_data->errcode = ecode;
	}

	if (MSP_SUCCESS == ecode)
		printf("更新词典成功！\n");
	else
		printf("更新词典失败！%d\n", ecode);

	return 0;
}

int update_lexicon(UserData *udata)
{
	const char *lex_content                   = "丁伟\n黄辣椒";
	unsigned int lex_cnt_len                  = strlen(lex_content);
	char update_lex_params[MAX_PARAMS_LEN]    = {NULL}; 

	snprintf(update_lex_params, MAX_PARAMS_LEN - 1, 
		"engine_type = local, text_encoding = UTF-8, \
		asr_res_path = %s, sample_rate = %d, \
		grm_build_path = %s, grammar_list = %s, ",
		ASR_RES_PATH,
		SAMPLE_RATE_16K,
		GRM_BUILD_PATH,
		udata->grammar_id);
	return QISRUpdateLexicon(LEX_NAME, lex_content, lex_cnt_len, update_lex_params, update_lex_cb, udata);
}





static void show_result(char *string, char is_over)
{
	printf("\rResult: [ %s ]", string);
	if(is_over)
		putchar('\n');
}

static char *g_result = NULL;
static unsigned int g_buffersize = BUFFER_SIZE;

void on_result(const char *result, char is_last)
{
	if (result) {
		size_t left = g_buffersize - 1 - strlen(g_result);
		size_t size = strlen(result);
		if (left < size) {
			g_result = (char*)realloc(g_result, g_buffersize + BUFFER_SIZE);
			if (g_result)
				g_buffersize += BUFFER_SIZE;
			else {
				printf("mem alloc failed\n");
				return;
			}
		}
		strncat(g_result, result, size);
		show_result(g_result, is_last);
	}
}
void on_speech_begin()
{
	if (g_result)
	{
		free(g_result);
	}
	g_result = (char*)malloc(BUFFER_SIZE);
	g_buffersize = BUFFER_SIZE;
	memset(g_result, 0, g_buffersize);

	printf("正在聆听...\n");
}
void on_speech_end(int reason)
{
	if (reason == END_REASON_VAD_DETECT)
		printf("\n聆听结束。 \n");
	else
		printf("\nRecognizer error %d\n", reason);
}

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
static void demo_mic(const char* session_begin_params,char* result)
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

int run_asr(UserData *udata,char * result)
{
	char asr_params[MAX_PARAMS_LEN]    = {NULL};
	const char *rec_rslt               = NULL;
	const char *session_id             = NULL;
	const char *asr_audiof             = NULL;
	FILE *f_pcm                        = NULL;
	char *pcm_data                     = NULL;
	long pcm_count                     = 0;
	long pcm_size                      = 0;
	int last_audio                     = 0;

	int aud_stat                       = MSP_AUDIO_SAMPLE_CONTINUE;
	int ep_status                      = MSP_EP_LOOKING_FOR_SPEECH;
	int rec_status                     = MSP_REC_STATUS_INCOMPLETE;
	int rss_status                     = MSP_REC_STATUS_INCOMPLETE;
	int errcode                        = -1;
	int aud_src                        = 0;
	//离线语法识别参数设置
	snprintf(asr_params, MAX_PARAMS_LEN - 1, 
		"engine_type = local, \
		asr_res_path = %s, sample_rate = %d, \
		asr_threshold = 40,asr_denoise=1,\
		grm_build_path = %s, local_grammar = %s, \
		result_type = json, result_encoding = UTF-8, ",
		ASR_RES_PATH,
		SAMPLE_RATE_16K,
		GRM_BUILD_PATH,
		udata->grammar_id
		);
	//printf("音频数据在哪? \n0: 从文件读入\n1:从MIC说话\n");
	//scanf("%d", &aud_src);
	//if(aud_src != 0) {
		demo_mic(asr_params,result);
	//} else {
	//	asr_audiof = get_audio_file();
	//	demo_file(asr_audiof, asr_params); 
	//}
	return 0;
}

int asr(char* bnf_file,char* slot_mode)
{
	int id=0;
	//const char *login_config = "appid = 5982f5e2,work_dir = ."; //登录参数
	UserData asr_data;
	int ret = 0;

	//ret = MSPLogin(NULL, NULL, login_config); //第一个参数为用户名，第二个参数为密码，传NULL即可，第三个参数是登录参数
	//printf("登录失败：%d\n", ret);
	//if (MSP_SUCCESS != ret && ret != 10132) {
		//printf("登录失败：%d\n", ret);
		//goto exit;
	//}

	memset(&asr_data, 0, sizeof(UserData));
	//printf("构建离线识别语法网络...\n");
	ret = build_grammar(&asr_data, bnf_file);  //第一次使用某语法进行识别，需要先构建语法网络，获取语法ID，之后使用此语法进行识别，无需再次构建
	if (MSP_SUCCESS != ret) {
		printf("构建语法调用失败！\n");
		goto exit;
	}
	while (1 != asr_data.build_fini)
		usleep(300);
	if (MSP_SUCCESS != asr_data.errcode)
		goto exit;
	//printf("离线识别语法网络构建完成，开始识别...\n");
	//printf("说出类似：我想参观一下汽车展厅 or 我想前往公司简介大厅参观一下 or 我想了解一下技术博物馆\n");
	char* result[2048] = {'\0'};
	memset(result, 0, sizeof(char) * 2048);
	ret = run_asr(&asr_data, result);
	cJSON *json = cJSON_Parse(result);
	//printf("result_json=%s", result_json);
	if (json) {
		int sn = cJSON_GetObjectItem(json, "sn")->valueint;
		cJSON *ws = cJSON_GetObjectItem(json, "ws");
		int size = 0;
		cJSON *arrayItem, *item;
		char* ws_child = NULL;
		size = cJSON_GetArraySize(ws);
		for (int i = 0; i < size; i++) {
			arrayItem = cJSON_GetArrayItem(ws, i);
			if (arrayItem) {
				ws_child = cJSON_Print(arrayItem);
				item = cJSON_Parse(ws_child);
				cJSON *slot_js = cJSON_GetObjectItem(item, "slot");
				char* slot = slot_js->valuestring;
				//printf("slot=%s", slot);
				//char *slot_str = cJSON_Print(slot);
				//±È½Ï×Ö·û´®£¬slot_modeÎª¹Ø¼üÃüÁî´ÊËùÊôµÄ²ÛÃû
				if (strcmp(slot, slot_mode)==0) {
					cJSON *cw_js = cJSON_GetObjectItem(item, "cw");
					char* cw_str = cJSON_Print(cw_js);
					//printf("cw_str=%s", cw_str);
					cJSON* cw_item = cJSON_Parse(cw_str);
					cJSON* cw_item_js = cJSON_GetArrayItem(cw_item, 0);
					cJSON *id_js = cJSON_GetObjectItem(cw_item_js, "id");
					cJSON *w_js = cJSON_GetObjectItem(cw_item_js, "w");
					//cJSON *id_child = cJSON_Print(id);

					id = id_js->valueint;
					char* w = w_js->valuestring;


				}
			}

		}
	}
	//char *place = cJSON_GetObjectItem(slot, "<place>")->valuestring;
	//printf("识别完成：%s", ws);

	if (MSP_SUCCESS != ret) {
		printf("离线语法识别出错: %d \n", ret);
		goto exit;
	}
	/*
	printf("请按任意键继续\n");
	_getch();
	
	printf("更新离线语法词典...\n");
	ret = update_lexicon(&asr_data);  //当语法词典槽中的词条需要更新时，调用QISRUpdateLexicon接口完成更新
	if (MSP_SUCCESS != ret) {
	printf("更新词典调用失败！\n");
	goto exit;
	}
	while (1 != asr_data.update_fini)
	_sleep(300);
	if (MSP_SUCCESS != asr_data.errcode)
	goto exit;
	printf("更新离线语法词典完成，开始识别...\n");
	
	ret = run_asr(&asr_data);
	if (MSP_SUCCESS != ret) {
		printf("离线语法识别出错: %d \n", ret);
		goto exit;
	}
	*/
exit:
	//MSPLogout();
	//printf("请按任意键退出...\n");
	//getchar();
	return id;
}






