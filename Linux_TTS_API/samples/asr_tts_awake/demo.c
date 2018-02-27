
#include"asr_tts_awake.h"




int main(int argc, char* argv[]){
	chdir(dirname(argv[0]));
	tts("开始运行");
		printf("Demo开始演示 ...\n");
	//char* text = "Demo开始演示 ...";
	tts("Demo程序开始运行 ...");
	sleep(2);
	tts("进入休眠状态");
	awake(1);
	//tts("")
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
		visit_chi:
			tts("你想前往哪一个项目或展厅进行参观呢？");
			printf("你可以对我说：\n1、我想前往公司简介大厅参观\n2、请带我参观一下技术博物馆\n3、我想参观一下汽车展厅\n");
			int id_ch_go = asr("goto.bnf", "<place>");//公司简介大厅!id(01)|技术博物馆!id(02)|汽车展厅!id(03);

			if (id_ch_go != 0) {
				/*
				char *place1_1,*place1_2,*place1_3;
				*place1_1 ="公司简介大厅";
				*place1_2 ="技术博物馆";
				*place1_3 ="汽车展厅";
				char**place1[]={place1_1,place1_2,place1_3};
				//std::string place1[] = { "公司简介大厅", "技术博物馆", "汽车展厅" };
				char* goal1 = "好的，将前往" + place1[id_ch_go - 1] + "进行参观";
				//const char* goal_char1 = goal1.c_str();
				tts(goal1);
				*/
				//开始参观
				//printf("开始参观");
				switch (id_ch_go) {
				case 1:
					tts("好的，将前往公司简介大厅进行参观");
					printf("按任意键到达目的地\n");
					getchar();
					printf("开始参观\n");
					tts("现在我们所在的地方为比亚迪公司简介大厅。比亚迪股份有限公司（简称“比亚迪”）是一家横跨IT、汽车、新能源和轻轨交通四大产业，并在香港和深圳两地上市的高新技术企业。比亚迪创立于1995年2月，从二次充电电池制造起步，2003年进入汽车行业，同时布局新能源产业，2016年3月进入轻轨交通产业。经过21年发展， 至2016年8月，比亚迪在全球共建立了26个生产基地， 总占地面积约1800万平方米，员工总数达22万人。");
					sleep(2);
					tts("进入休眠状态");
					awake(1);
					tts("我们继续参观");
					goto visit_chi;
					break;
				case 2:
					tts("好的，将前往技术博物馆进行参观");
					printf("按任意键到达目的地\n");
					getchar();
					printf("开始参观\n");
					tts("现在参观的是比亚迪技术博物馆。比亚迪技术博物馆主要展示了比亚迪发展至今所研究的技术领域，包括对五大研究院的介绍，垂直整合一站式服务的介绍，和产品的介绍。");
					sleep(2);
					tts("进入休眠状态");
					awake(1);
					tts("我们继续参观");
					goto visit_chi;
					break;
				case 3:
					tts("好的，将前往汽车展厅进行参观");
					printf("按任意键到达目的地\n");
					getchar();
					printf("开始参观\n");
					tts("现在我们来到的是比亚迪汽车展厅。比亚迪于2003年收购陕西秦川汽车有限责任公司，组建比亚迪汽车，正式进入汽车制造和销售行业。在汽车展厅，您将看到比亚迪目前生产的多种车型，首先映入眼帘的是e6。");
					sleep(2);
					tts("进入休眠状态");
					awake(1);
					tts("我们继续参观");
					goto visit_chi;
					break;
				default:break;
				}
				printf("按任意键结束本次服务\n");
				getchar();
				tts("本次服务到此结束，祝您度过美好的一天.");
			}
			else {
				tts("我没有听清。");
				goto visit_chi;
			}
		}else if(id_mode==2){
			tts("The language mode was successfully set to English");
			tts("Hello, welcome to the B Y D exhibition hall, I'm the guide robot 小迪, happy to serve you.");
		visit_eng:
			tts("What kind of project or exhibition do you want to visit?");
			int id_en_go = asr("goto_eng.bnf", "<place>");//公司简介大厅!id(01)|技术博物馆!id(02)|汽车展厅!id(03);
			
			if (id_en_go != 0) {

				/*
				char *place2_1,*place2_2,*place2_3;
				*place2_1 ="Company Profile Area";
				*place2_2 ="Technology Museum";
				*place2_3 ="Auto Showroom";
				char**place2[]={place1_1,place1_2,place1_3};
				//std::string place1[] = { "公司简介大厅", "技术博物馆", "汽车展厅" };
				char* goal2 = "OK,we'll visit the" + place2[id_ch_go - 1];
				//const char* goal_char1 = goal1.c_str();
				tts(goal2);
				//开始参观
				printf("Start a visit");
				*/
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
		goto hello;
	}

	
	return 0;

}