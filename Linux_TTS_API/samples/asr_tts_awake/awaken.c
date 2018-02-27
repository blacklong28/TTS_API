#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include"asr_tts_awake.h"
/* use the newer alsa api */
#define alsa_pcm_new_hw_params_api
#include <alsa/asoundlib.h>
#include "../../include/msp_cmn.h"
#include "../../include/qivw.h"
#include "../../include/msp_errors.h"

#define FRAME_LEN	640 //16k采样率的16bit音频，一帧的大小为640B, 时长20ms
int GETOUT=0;

void sleep_ms(int ms)
{
	usleep(ms * 1000);
}

int cb_ivw_msg_proc( const char *sessionID, int msg, int param1, int param2, const void *info, void *userData )
{
	if (MSP_IVW_MSG_ERROR == msg) //唤醒出错消息
	{
		printf("\n\nMSP_IVW_MSG_ERROR errCode = %d\n\n", param1);
	}
	else if (MSP_IVW_MSG_WAKEUP == msg) //唤醒成功消息
	{
		printf("\n\nMSP_IVW_MSG_WAKEUP result = %s\n\n", info);
		printf ("检测到唤醒词\n");
		GETOUT=1;
		//goto exit;
		//break;
	}
	return 0;
}

void run_ivw(const char *grammar_list, const char* session_begin_params)
{
	const char *session_id = NULL;
	int err_code = MSP_SUCCESS;
	FILE *f_aud = NULL;
	long audio_size = 0;
	long real_read = 0;
	long audio_count = 0;
	int count = 0;
	int audio_stat = MSP_AUDIO_SAMPLE_CONTINUE;
	char *audio_buffer=NULL;
	char sse_hints[128];
	int rc;
	snd_pcm_t *handle;
	snd_pcm_hw_params_t *params;
	unsigned int val;
	int dir;
	snd_pcm_uframes_t frames;
	char *buffer;
	//int GETOUT=0;
#if 1
	/* Open PCM device for recording (capture). */
	rc = snd_pcm_open(&handle, "default",
			SND_PCM_STREAM_CAPTURE, 0);
	if (rc < 0) {
		fprintf(stderr,
				"unable to open pcm device: %s\n",
				snd_strerror(rc));
		exit(1);
	}

	/* Allocate a hardware parameters object. */
	snd_pcm_hw_params_alloca(&params);

	/* Fill it in with default values. */
	snd_pcm_hw_params_any(handle, params);

	/* Set the desired hardware parameters. */

	/* Interleaved mode */
	//snd_pcm_hw_params_set_access(handle, params,
	//                      SND_PCM_ACCESS_RW_NONINTERLEAVED);

	/* Signed 16-bit little-endian format */
	snd_pcm_hw_params_set_format(handle, params,
			SND_PCM_FORMAT_S16_LE);

	/* Two channels (stereo) */
	snd_pcm_hw_params_set_channels(handle, params, 1);

	/* 44100 bits/second sampling rate (CD quality) */
	val = 16000;
	snd_pcm_hw_params_set_rate_near(handle, params,
			&val, &dir);

	/* Set period size to 32 frames. */
	frames = FRAME_LEN;
	snd_pcm_hw_params_set_period_size_near(handle,
			params, &frames, &dir);

	/* Write the parameters to the driver */
	rc = snd_pcm_hw_params(handle, params);
	if (rc < 0) {
		fprintf(stderr,
				"unable to set hw parameters: %s\n",
				snd_strerror(rc));
		exit(1);
	}

	audio_size = frames * 2; /* 2 bytes/sample, 2 channels */
	audio_buffer = (char *) malloc(audio_size);

	/* We want to loop for 5 seconds */
	snd_pcm_hw_params_get_period_time(params, &val, &dir);
	printf("开始休眠\n");
	printf("请缓慢清晰的说出唤醒词:\n1、小 迪，你 好\n2、你 好，小 迪\n3、小 迪，在 吗\n");
	//printf ("priod time is %u\n", val);
	//printf ("fulinux %d\n", __LINE__);
#else
	if (NULL == audio_filename)
	{
		printf("params error\n");
		return;
	}

	f_aud=fopen(audio_filename, "rb");
	if (NULL == f_aud)
	{
		printf("audio file open failed! \n");
		return;
	}
	fseek(f_aud, 0, SEEK_END);
	audio_size = ftell(f_aud);
	fseek(f_aud, 0, SEEK_SET);
	audio_buffer = (char *)malloc(audio_size);
	if (NULL == audio_buffer)
	{
		printf("malloc failed! \n");
		goto exit;
	}
	real_read = fread((void *)audio_buffer, 1, audio_size, f_aud);
	if (real_read != audio_size)
	{
		printf("read audio file failed!\n");
		goto exit;
	}
#endif
begin:
	//printf ("fulinux %d\n", __LINE__);
	if(byd_session_id_ivw !=NULL){
		printf("session_id !=NULL");
		QIVWSessionEnd(byd_session_id_ivw, "byd_exit3");
	}

	session_id=QIVWSessionBegin(grammar_list, session_begin_params, &err_code);



	if (err_code != MSP_SUCCESS)
	{
		printf("QIVWSessionBegin failed! error code:%d\n",err_code);

		/*
		if(err_code==10132){
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
			goto exit;
		}
		*/

			goto exit;
	
	}


	byd_session_id_ivw = session_id;
	//api_id = 3;
	//printf ("fulinux %d\n", __LINE__);
	err_code = QIVWRegisterNotify(session_id, cb_ivw_msg_proc,NULL);
	if (err_code != MSP_SUCCESS)
	{
		snprintf(sse_hints, sizeof(sse_hints), "QIVWRegisterNotify errorCode=%d", err_code);
		printf("QIVWRegisterNotify failed! error code:%d\n",err_code);
		goto exit;
	}

	//printf ("fulinux %d\n", __LINE__);
	
	while(GETOUT==0)
	{
		audio_stat = MSP_AUDIO_SAMPLE_LAST; //最后一块
		//printf("snd_pcm_readi--%d\n",k);
		//k++;
		rc = snd_pcm_readi(handle, audio_buffer, frames);
		if (rc == -EPIPE) {
			/* EPIPE means overrun */
			fprintf(stderr, "overrun occurred\n");
			snd_pcm_prepare(handle);
		} else if (rc < 0) {
			fprintf(stderr,
					"error from read: %s\n",
					snd_strerror(rc));
		} else if (rc != (int)frames) {
			fprintf(stderr, "short read, read %d frames\n", rc);
		}

		err_code = QIVWAudioWrite(session_id, (const void *)&audio_buffer[0], frames, audio_stat);
		if (MSP_SUCCESS != err_code)
		{
			printf("QIVWAudioWrite failed! error code:%d\n",err_code);
			snprintf(sse_hints, sizeof(sse_hints), "QIVWAudioWrite errorCode=%d", err_code);
			goto exit;
		}
	}
	snprintf(sse_hints, sizeof(sse_hints), "success");

exit:
	if (NULL != session_id)
	{
		QIVWSessionEnd(session_id, sse_hints);
	}
	if (NULL != f_aud)
	{
		fclose(f_aud);
	}
	if (NULL != audio_buffer)
	{
		free(audio_buffer);
	}
}


int awake(int id)
{
	const char *ssb_param = "engine_start = ivw,ivw_res_path =fo|res/ivw/wakeupresource.jet,ivw_threshold=0:-10;1:-10;2:-10,sst=wakeup";
	//const char * byd_param = "engine_start = ivw,ivw_res_path =fo|res/ivw/wakeupresource.jet";
	run_ivw(NULL, ssb_param); 
exit:
	//printf("按任意键退出 ...\n");
	//getchar();
	GETOUT=0;
	//MSPLogout(); //退出登录
	return 0;
}
