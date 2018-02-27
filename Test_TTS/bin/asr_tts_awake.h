
int logOut();
int initTTS();
//awake
/**
**语音唤醒部分接口函数
**/
void sleep_ms(int ms);

int cb_ivw_msg_proc( const char *sessionID, int msg, int param1, int param2, const void *info, void *userData );

void run_ivw(const char *grammar_list, const char* session_begin_params);//进入语音唤醒等待状态

int awake(int id);//进入休眠状态，返回int值为0,暂时没有其他返回值；输入参数id为保留参数，暂时没有意义。
int stopAwake();

//char* session_id_byd;
//int api_id=0;//tts = 1;asr = 2;awake = 3;

//tts
/**
**语音合成相关接口
**/ 
typedef int SR_DWORD;
typedef short int SR_WORD ;

/* wav音频头部格式 */
typedef struct _wave_pcm_hdr
{
	char            riff[4];                // = "RIFF"
	int				size_8;                 // = FileSize - 8
	char            wave[4];                // = "WAVE"
	char            fmt[4];                 // = "fmt "
	int				fmt_size;				// = 下一个结构体的大小 : 16

	short int       format_tag;             // = PCM : 1
	short int       channels;               // = 通道数 : 1
	int				samples_per_sec;        // = 采样率 : 8000 | 6000 | 11025 | 16000
	int				avg_bytes_per_sec;      // = 每秒字节数 : samples_per_sec * bits_per_sample / 8
	short int       block_align;            // = 每采样点字节数 : wBitsPerSample / 8
	short int       bits_per_sample;        // = 量化比特数: 8 | 16

	char            data[4];                // = "data";
	int				data_size;              // = 纯数据长度 : FileSize - 44 
} wave_pcm_hdr;

int text_to_speech(const char* src_text, const char* des_path, const char* params);//语音文件生成接口

int tts(char* text);//语音文件生成+播放接口

//asr
/**
**离线命令词识别相关接口
**/
#define FRAME_LEN	640 
#define	BUFFER_SIZE	4096
#define SAMPLE_RATE_16K     (16000)
#define SAMPLE_RATE_8K      (8000)
#define MAX_GRAMMARID_LEN   (32)
#define MAX_PARAMS_LEN      (1024)
typedef struct _UserData {
	int     build_fini; //标识语法构建是否完成
	int     update_fini; //标识更新词典是否完成
	int     errcode; //记录语法构建或更新词典回调错误码
	char    grammar_id[MAX_GRAMMARID_LEN]; //保存语法构建返回的语法ID
}UserData;


void on_result(const char *result, char is_last);
void on_speech_begin();
void on_speech_end(int reason);


const char *get_audio_file(void); //选择进行离线语法识别的语音文件

int build_grammar(UserData *udata,char*grm_file); //构建离线识别语法网络

int update_lexicon(UserData *udata); //更新离线识别语法词典

int run_asr(UserData *udata,char*result); //进行离线语法识别

/* 离线命令词识别主要入口，bnf_file为bnf语法文件名称，slot_mode为需要识别的命令词槽名，例如：“<place>” */
int asr(char* bnf_file,char* slot_mode);



//iat +sch
int sch(char*iat_result,const char*tts_result);
int iat(char*text);
