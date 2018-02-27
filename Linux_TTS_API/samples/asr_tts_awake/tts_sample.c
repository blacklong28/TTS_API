/*
* 语音合成（Text To Speech，TTS）技术能够自动将任意文字实时转换为连续的
* 自然语音，是一种能够在任何时间、任何地点，向任何人提供语音信息服务的
* 高效便捷手段，非常符合信息时代海量数据、动态更新和个性化查询的需求。
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

/* use the newer alsa api */
#define alsa_pcm_new_hw_params_api
#include <alsa/asoundlib.h>

//不同的声卡有着不同的播放参数，这些参数可以使用file命令获得

#define Sample_Size 16 //there're two kinds of bits,8 bits and 16 bits

#define Sample_Rate 8000 //sampling rate

#include "../../include/qtts.h"
#include "../../include/msp_cmn.h"
#include "../../include/msp_errors.h"
#include "asr_tts_awake.h"
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

/* Ä¬ÈÏwavÒôÆµÍ·²¿Êý¾Ý */
wave_pcm_hdr default_wav_hdr = 
{
	{ 'R', 'I', 'F', 'F' },
	0,
	{'W', 'A', 'V', 'E'},
	{'f', 'm', 't', ' '},
	16,
	1,
	1,
	16000,
	32000,
	2,
	16,
	{'d', 'a', 't', 'a'},
	0  
};

/* 文本合成 */
int text_to_speech(const char* src_text, const char* des_path, const char* params)
{
	int          ret          = -1;
	FILE*        fp           = NULL;
	const char*  sessionID    = NULL;
	unsigned int audio_len    = 0;
	wave_pcm_hdr wav_hdr      = default_wav_hdr;
	int          synth_status = MSP_TTS_FLAG_STILL_HAVE_DATA;

	if (NULL == src_text || NULL == des_path)
	{
		printf("params is error!\n");
		return ret;
	}
	fp = fopen(des_path, "wb");
	if (NULL == fp)
	{
		printf("open %s error.\n", des_path);
		return ret;
	}
	/* 开始合成 */

	if(byd_session_id_tts != NULL){
		QTTSSessionEnd(byd_session_id_tts, "byd_exit1");
		printf("sessionID!=NULL\n");
	}
begin:
	sessionID = QTTSSessionBegin(params, &ret);
	if (MSP_SUCCESS != ret)
	{
		/*
		printf("QTTSSessionBegin failed, error code: %d.\n", ret);
		if(ret==10132){
			if(api_id==1){
				printf("QTTSSessionEnd api_id == 1\n");
				QTTSSessionEnd(session_id_byd, "byd_exit1");
			}else if(api_id==2){
				printf("QISRSessionEnd api_id == 2\n");
				QISRSessionEnd(session_id_byd, "byd_exit2");
			}else if(api_id==3){
				printf("QIVWSessionEnd api_id == 3\n");
				QIVWSessionEnd(session_id_byd, "byd_exit3");
			}
			goto begin;
		}else{
			fclose(fp);
			return ret;
		}
		*/

		//if(ret==10132){
		//	QTTSSessionEnd(sessionID, "byd_exit1");
			//QISRSessionEnd(sessionID, "byd_exit2");
			//QIVWSessionEnd(sessionID, "byd_exit3");
			//goto begin;
		//}else{
			fclose(fp);
			return ret;
			
		//}
	}
	byd_session_id_tts = sessionID;
	//api_id = 1;
	ret = QTTSTextPut(sessionID, src_text, (unsigned int)strlen(src_text), NULL);
	if (MSP_SUCCESS != ret)
	{
		printf("QTTSTextPut failed, error code: %d.\n",ret);
		QTTSSessionEnd(sessionID, "TextPutError");
		fclose(fp);
		return ret;
	}
	//printf("正在合成 ...\n");
	fwrite(&wav_hdr, sizeof(wav_hdr) ,1, fp); //添加wav音频头，使用采样率为16000
	while (1) 
	{
		/* 获取合成音频 */
		const void* data = QTTSAudioGet(sessionID, &audio_len, &synth_status, &ret);
		if (MSP_SUCCESS != ret)
			break;
		if (NULL != data)
		{
			fwrite(data, audio_len, 1, fp);
		    wav_hdr.data_size += audio_len; //计算data_size大小
		}
		if (MSP_TTS_FLAG_DATA_END == synth_status)
			break;
	}
	printf("\n");
	if (MSP_SUCCESS != ret)
	{
		printf("QTTSAudioGet failed, error code: %d.\n",ret);
		QTTSSessionEnd(sessionID, "AudioGetError");
		fclose(fp);
		return ret;
	}
	/* 修正wav文件头数据的大小 */
	wav_hdr.size_8 += wav_hdr.data_size + (sizeof(wav_hdr) - 8);
	
	/* 将修正过的数据写回文件头部,音频文件为wav格式 */
	fseek(fp, 4, 0);
	fwrite(&wav_hdr.size_8,sizeof(wav_hdr.size_8), 1, fp); //写入size_8的值
	fseek(fp, 40, 0); //将文件指针偏移到存储data_size值的位置
	fwrite(&wav_hdr.data_size,sizeof(wav_hdr.data_size), 1, fp); //写入data_size的值
	fclose(fp);
	fp = NULL;
	/* 合成完毕 */
	ret = QTTSSessionEnd(sessionID, "Normal");
	if (MSP_SUCCESS != ret)
	{
		printf("QTTSSessionEnd failed, error code: %d.\n",ret);
	}

	return ret;
}

int tts(char* text)
{
	int         ret                  = MSP_SUCCESS;
	//const char* login_params         = "appid = 5982f5e2, work_dir = .";//登录参数,appid与msc库绑定,请勿随意改动
	/*
	* rdn:           合成音频数字发音方式
	* volume:        合成音频的音量
	* pitch:         合成音频的音调
	* speed:         合成音频对应的语速
	* voice_name:    合成发音人
	* sample_rate:   合成音频采样率
	* text_encoding: 合成文本编码格式
	*
	* 详细参数说明请参阅《iFlytek MSC Reference Manual》
	*/
	const char* session_begin_params = "engine_type = local, text_encoding = UTF8, tts_res_path = fo|res/tts/xiaoyan.jet;fo|res/tts/common.jet, sample_rate = 16000, speed = 50, volume = 50, pitch = 50, rdn = 2,rcn =1";
	const char* filename             = "tts_result.wav"; //合成的语音文件名称
	//const char* text                 = "亲爱的用户，您好，这是一个语音合成示例，感谢您对科大讯飞语音技术的支持！科大讯飞是亚太地区最大的语音上市公司，股票代码：002230"; //合成文本
	/* 用户登录 */
	//ret = MSPLogin(NULL, NULL, login_params); //第一个参数是用户名，第二个参数是密码，第三个参数是登录参数，用户名和密码可在http://www.xfyun.cn注册获取
	//printf("MSPLogin failed, error code: %d.\n", ret);
	//if (MSP_SUCCESS != ret && ret != 10132)
	//{
	//	printf("MSPLogin failed, error code: %d.\n", ret);
	//	goto exit ;//登录失败，退出登录
	//}

	//printf("\n###########################################################################\n");
	//printf("## 语音合成（Text To Speech，TTS）技术能够自动将任意文字实时转换为连续的 ##\n");
	//printf("## 自然语音，是一种能够在任何时间、任何地点，向任何人提供语音信息服务的  ##\n");
	//printf("## 高效便捷手段，非常符合信息时代海量数据、动态更新和个性化查询的需求。  ##\n");
	//printf("###########################################################################\n\n");

	/* 文本合成 */
	//printf("开始合成 ...\n");
	ret = text_to_speech(text, filename, session_begin_params);
	if (MSP_SUCCESS != ret)
	{
		printf("text_to_speech failed, error code: %d.\n", ret);
	}
	if(ret == MSP_SUCCESS){
		FILE *fp;
		int nread;
		fp = fopen("./tts_result.wav","rb");
		if(fp==NULL){
			perror("open file failed:\n");
		}else{
			nread=fread(&wav_header,1,sizeof(wav_header),fp);
			set_pcm_play(fp);
			//printf("play success!\n");
		}

	}
	//printf("合成完毕\n");

exit:
	//printf("按任意键退出 ...\n");
	//getchar();
	//MSPLogout(); //退出登录

	return 0;
}



 int set_pcm_play(FILE *fp)
{
        int rc;
        int ret;
        int size;
        snd_pcm_t* handle; //PCI设备句柄
        snd_pcm_hw_params_t* params;//硬件信息和PCM流配置
        unsigned int val;
        int dir=0;
        snd_pcm_uframes_t frames;
        char *buffer;
        int channels=wav_header.wChannels;
        int frequency=wav_header.nSamplesPersec;
        int bit=wav_header.wBitsPerSample;
        int datablock=wav_header.wBlockAlign;
        unsigned char ch[100]; //用来存储wav文件的头信息
    
    
        
        rc=snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
        if(rc<0)
        {
                perror("\nopen PCM device failed:");
                exit(1);
        }


        snd_pcm_hw_params_alloca(&params); //分配params结构体
        if(rc<0)
        {
                perror("\nsnd_pcm_hw_params_alloca:");
                exit(1);
        }
         rc=snd_pcm_hw_params_any(handle, params);//初始化params
        if(rc<0)
        {
                perror("\nsnd_pcm_hw_params_any:");
                exit(1);
        }
        rc=snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED); //初始化访问权限
        if(rc<0)
        {
                perror("\nsed_pcm_hw_set_access:");
                exit(1);

        }

        //采样位数
        switch(bit/8)
        {
        case 1:snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_U8);
                break ;
        case 2:snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
                break ;
        case 3:snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S24_LE);
                break ;

        }
        rc=snd_pcm_hw_params_set_channels(handle, params, channels); //设置声道,1表示单声>道，2表示立体声
        if(rc<0)
        {
                perror("\nsnd_pcm_hw_params_set_channels:");
                exit(1);
        }
        val = frequency;
        rc=snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir); //设置>频率
        if(rc<0)
        {
                perror("\nsnd_pcm_hw_params_set_rate_near:");
                exit(1);
        }

        rc = snd_pcm_hw_params(handle, params);
        if(rc<0)
        {
        perror("\nsnd_pcm_hw_params: ");
        exit(1);
        }

        rc=snd_pcm_hw_params_get_period_size(params, &frames, &dir); /*获取周期
长度*/
        if(rc<0)
        {
                perror("\nsnd_pcm_hw_params_get_period_size:");
                exit(1);
        }

        size = frames * datablock; /*4 代表数据快长度*/

        buffer =(char*)malloc(size);
    fseek(fp,58,SEEK_SET); //定位歌曲到数据区

    while (1)
        {
                memset(buffer,0,sizeof(buffer));
                ret = fread(buffer, 1, size, fp);
                if(ret == 0)
                {
                        //printf("音频写入结束\n");
                        break;
                }
                 else if (ret != size)
                {
                 }
                // 写音频数据到PCM设备
        while(ret = snd_pcm_writei(handle, buffer, frames)<0)
           {
                 usleep(2000);
                 if (ret == -EPIPE)
                {
                  /* EPIPE means underrun */
                  fprintf(stderr, "underrun occurred\n");
                  //完成硬件参数设置，使设备准备好
                  snd_pcm_prepare(handle);
                 }
                 else if (ret < 0)
                 {
                          fprintf(stderr,
                      "error from writei: %s\n",
                      snd_strerror(ret));
                 }
            }

    }

        snd_pcm_drain(handle);
        snd_pcm_close(handle);
        free(buffer);
        //printf("play return \n");
        return 0;
} 

int initTTS(){
	int         ret       = MSP_SUCCESS;
	const char *lgi_param = "appid = 5982f5e2,work_dir = ."; 
	ret = MSPLogin(NULL, NULL, lgi_param);
	printf("MSPLogin , return code: %d.\n", ret);
	if (MSP_SUCCESS != ret)
	{
		printf("MSPLogin failed, error code: %d.\n", ret);
		return ret ;//登录失败，退出登录
	}
}
int logOut(){
	MSPLogout();
}