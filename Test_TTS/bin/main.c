
#include"asr_tts_awake.h"

#include<stdio.h>  
#include<pthread.h>  
#include<string.h>  
#include<sys/types.h>  
#include<unistd.h>
#include <errno.h>  
pthread_t ntid,ntid2;  

void printids(const char *s)  
{  
    pid_t pid;  
    pthread_t tid ;  
    pid = getpid();  
    tid = pthread_self();  
    printf("%s pid %u tid %u (0x%x)\n",s,(unsigned int)pid,(unsigned int)tid,(unsigned int)tid);  
  
}  
  
void *thread(void *arg)  
{  
    printids("new thread:");
	tts("子线程进入休眠\n");
	awake(1);
	printf("休眠结束\n");
    return ((void *)0);  
}


int main(int argc, char* argv[]){
	//chdir(dirname(argv[0]));

	int ret = initTTS();
	printf("login ret=%d\n",ret);

	//tts("开始运行!!");
	printf("Demo程序开始运行 ...\n");

    int temp;
    if((temp=pthread_create(&ntid,NULL,thread,NULL))!= 0)  
    {  
        printf("can't create thread: %s\n",strerror(temp));  
        return 1;  
     }  
     printids("main thread:");  
     sleep(8);
	 while(1){
		 
		pthread_cancel(ntid);
		int kill_rc = pthread_kill(ntid,0);
		if(kill_rc == ESRCH){
		printf("the specified thread did not exists or already quit\n");
		    if((temp=pthread_create(&ntid2,NULL,thread,NULL))!= 0)  
    		{  
				printf("can't create thread: %s\n",strerror(temp));  
				return 1;  
     		}else {

				 sleep(8);
				 return 0;
			 }
	 	}
		else if(kill_rc == EINVAL)
		printf("signal is invalid\n");
		else
		printf("the specified thread is alive\n");
		//printf("已经杀死子线程！！\n");
	 }  
     return 0;  






	//char* text = "Demo开始演示 ...";
	tts("Demo程序开始运行 ...");
	sleep(2);//暂停2秒，准备进入休眠

	/*
	tts("你有什么要和我说的吗？");
	awake(1);

	char* result[2048] = {'\0'};
	memset(result, 0, sizeof(char) * 2048);
	iat(result);
	printf("result==%s\n",result);
	char* tts_result[2048] = {'\0'};
	memset(tts_result, 0, sizeof(char) * 2048);
	sch(result,tts_result);
	printf("tts_result==%s\n",tts_result);
	tts(tts_result);

	tts("不和你闲聊了，下面进行导览服务！");
	*/
	int lan_times = 0;
	int place_time =0;
hello:
	tts("使用中文模式请对我说：你好，小迪；In English, please say to me: hi，小迪.");
	printf("说出：你好，小迪 or hi，Xiaodi.\n");
	int id_mode = asr("goto.bnf","<lanmode>");
	char *language_1,*language_2;
	char **language[]={language_1,language_2};
	//std::string language[] = { "中文", "English" };
	if (id_mode != 0){
		if (id_mode == 1) {
			tts("语言模式成功设置为中文");
			//sleep(8);
			
			tts("您好，欢迎来到比亚迪展厅，我是导览机器人小迪，很高兴为你服务。");
			int place_ago1 =0;
			int place_ago2 =0;
			int place_ago3 =0;
		visit_chi:
			tts("你想前往哪一个项目或展厅进行参观呢？");
			printf("你可以对我说：\n1、我想前往<公司简介大厅>参观\n2、请带我参观一下<技术博物馆>\n3、我想参观一下<汽车展厅>\n");
			int id_ch_go = asr("goto.bnf", "<place>");//公司简介大厅!id(01)|技术博物馆!id(02)|汽车展厅!id(03);

			if (id_ch_go != 0) {
				switch (id_ch_go) {
				case 1:
					tts("好的，将前往公司简介大厅进行参观");
					printf("按Enter键到达目的地\n");
					getchar();
					printf("开始参观\n");
					tts("现在我们所在的地方为比亚迪公司简介大厅。比亚迪股份有限公司（简称“比亚迪”）是一家横跨IT、汽车、新能源和轻轨交通四大产业，并在香港和深圳两地上市的高新技术企业。");
					tts("比亚迪创立于1995年2月，从二次充电电池制造起步，2003年进入汽车行业，同时布局新能源产业，2016年3月进入轻轨交通产业。");
					sleep(2);
					tts("进入休眠状态");
					place_ago1 =1;//到此一游
					awake(1);
					if(place_ago1+place_ago2+place_ago3 ==3){
						goto exit;
					}
					tts("我们继续参观");
					goto visit_chi;
					break;
				case 2:
					tts("好的，将前往技术博物馆进行参观");
					printf("按Enter键到达目的地\n");
					getchar();
					printf("开始参观\n");
					tts("现在参观的是比亚迪技术博物馆。比亚迪技术博物馆主要展示了比亚迪发展至今所研究的技术领域，包括对五大研究院的介绍，垂直整合一站式服务的介绍，和产品的介绍。");
					sleep(2);
					tts("进入休眠状态");
					place_ago2 =1;//到此一游
					awake(1);
					if(place_ago1+place_ago2+place_ago3 ==3){
						goto exit;
					}
					tts("我们继续参观");
					goto visit_chi;
					break;
				case 3:
					tts("好的，将前往汽车展厅进行参观");
					printf("按Enter键到达目的地\n");
					getchar();
					printf("开始参观\n");
					tts("现在我们来到的是比亚迪汽车展厅。比亚迪于2003年收购陕西秦川汽车有限责任公司，组建比亚迪汽车，正式进入汽车制造和销售行业。在汽车展厅，您将看到比亚迪目前生产的多种车型，首先映入眼帘的是e6。");
					sleep(2);
					tts("进入休眠状态");
					place_ago3 =1;//到此一游
					awake(1);
					if(place_ago1+place_ago2+place_ago3 ==3){
						goto exit;
					}
					tts("我们继续参观");
					goto visit_chi;
					break;
				default:break;
				}
		exit:	
				tts("我们已经把所有展厅参观一遍了");
				printf("按Enter键结束本次服务\n");
				getchar();
				tts("本次服务到此结束，祝您度过美好的一天.");
			}
			else {
				tts("我没有听清。");
				place_time=place_time+1;
				if(place_time>=2){
					tts("我问累了，我要休息一下！");
					awake(1);
					place_time =0;
				}
				goto visit_chi;
			}
		}else if(id_mode==2){
			tts("The language mode was successfully set to English");
			tts("Hello, welcome to the B Y D exhibition hall, I'm the guide robot 小迪, happy to serve you.");
			tts("英语模式不支持离线命令词识别，请进入中文模式进行功能的体验。");
			goto hello;
		
		visit_eng:
			tts("What kind of project or exhibition do you want to visit?");
			int id_en_go = asr("goto_eng.bnf", "<place>");//公司简介大厅!id(01)|技术博物馆!id(02)|汽车展厅!id(03);
			
			if (id_en_go != 0) {

				switch (id_en_go) {
				case 1:
					tts("OK,we'll visit the Company Profile Area");
					printf("Start a visit");
					tts("Now,we are in the Company Profile Area. B Y D Company Limited is a high-tech enterprise specializing in four industries: IT, Automobile, New Energy and Light Rail. The company is listed on the Hong Kong and Shenzhen Stock Exchanges.Founded in February 1995, B Y D started out in the rechargeable battery business, and entered the automobile industry in 2003. In the meantime, it started operating in the renewable energy industry.In March 2016, B Y D ventured into the light rail industry.By August 2016, B Y D had a staff of 220, 000 employees in 26 industrial parks around the world, covering an area of approximately 18 million sq.m.");
					break;
				case 2:
					tts("OK,we'll visit the Technology Museum");
					printf("Start a visit");
					tts("Now,we are in the Technology Museum. B Y D Technology Museum mainly shows the development of B Y D has been the technology field, including the introduction of the five research institutes, vertical integration of one-stop service introduction, and product introduction.");
					break;
				case 3:
					tts("OK,we'll visit the Auto Showroom");
					printf("Start a visit");
					tts("Now,we are in the Auto Showroom.B Y D formally entered auto manufacture and sales field after having purchased Tsinchuan Automobile Company Limited in Shanxi Province in 2003 and found B Y D Auto Company Limited. You will see the current models of B Y D in auto showroom. The first is e6.");
					break;
				default:
					break;
				}
				tts("At the end of this service, I wish you a wonderful day.");
			}
			else {
				tts("I did not hear clearly.");
				goto visit_eng;
			}
		}
	}
	else {
		tts("我没有听清。");
		lan_times=lan_times+1;
		if(lan_times>=2){
			tts("我问累了，我要休息一下！");
			awake(1);
			lan_times=0;
		}
		goto hello;
	}

	
	return 0;

}
