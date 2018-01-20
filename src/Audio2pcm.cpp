#include "Audio2pcm.h"

typedef std::string::size_type string_size;
static void SplitString(const std::string& s, std::vector<std::string>& v, const std::string& c);

void Audio2Pcm::HandleVoiceType_pcm()
{
	//128kbps的pcm 
	if(decodePara->channel == CHANNEL_MONO)   
	{
		 //pcm单声道语音重命名
		string command("cp \"");
		command+=inWavName+"\" \""+outWavName+"\"";
		system(command.c_str());
	}
	else if(decodePara->channel == CHANNEL_STEREO)
	{
		if(decodePara->stereOnMode == STEREO_ON_1)       //将分录的双声道语音合并成一个合录的单声道语音
		{
			//SplitChannel:pcm->pcm---mix
			string command("./SplitChannel -f \"");
			command+=inWavName+"\"  -rate 8000 -om \""+outWavName+"\"";
			system(command.c_str());			
		}
		else if(decodePara->stereOnMode == STEREO_ON_2)   //将分录双声道的左右声道解码成两个单声道
		{
			//SplitChannel:pcm->pcm---left,right
			string command("./SplitChannel -f \"");
			command+=inWavName+"\"  -rate 8000 -ol \""+outWavName_left+"\" -or  \""+outWavName_right+"\"";
			system(command.c_str());
		}
		else if(decodePara->stereOnMode == STEREO_ON_3)   //将分录双声道的左右声道解码成两个单声道同时，还需合并成一个单声道
		{
			//SplitChannel:pcm->pcm---left,right,mix
			string command("./SplitChannel -f \"");
			command+=inWavName+"\"  -rate 8000 -ol \""+outWavName_left+"\" -or  \""+outWavName_right+"\" -om \""+outWavName+"\"";
			system(command.c_str());			
		}
		else if(decodePara->stereOnMode == STEREO_ON_4)     //合录双声道，左右声道内容一样，都是多个人的对话，则仅保留一个声道内容
		{
			//SplitChannel:pcm->pcm---ethier
			string command("./SplitChannel -f \"");
			command+=inWavName+"\"  -rate 8000 -ol \""+outWavName+"\"";
			system(command.c_str());
		}
	}
}

void Audio2Pcm::HandleVoiceType_pcm_spec()
{
    //采样率为非8k的pcm
    if(decodePara->channel == CHANNEL_MONO)   
    {
        //resample:pcm_spec->pcm
        string command("./resample -to 8000 \"");
        command+=inWavName+"\"  \""+outWavName+"\"";
        system(command.c_str());
    }
    else if(decodePara->channel == CHANNEL_STEREO)
    {   
        if(decodePara->stereOnMode == STEREO_ON_2 || decodePara->stereOnMode == STEREO_ON_3)
        {
            string tempLeft=outWavName;
            size_t dotpos1=tempLeft.rfind(".");
            int replaceLen1=tempLeft.length()-dotpos1;
            tempLeft.replace(dotpos1,replaceLen1,"_left_temp.wav");
            
            string tempright=outWavName;
            size_t dotpos2=tempright.rfind(".");
            int replaceLen2=tempright.length()-dotpos2;
            tempright.replace(dotpos2,replaceLen2,"_right_temp.wav");
            
            //sox:pcm_spec->pcm---left
            // string command1("sox \"");
            // command1+=inWavName+"\"  \""+tempLeft+"\" remix 1";
			string command1("sox ");
            command1+=inWavName+"  "+tempLeft+" remix 1";
            CallCodecFunction(command1);
            
            //resample:pcm_spec->pcm---left
            string command2("./resample -to 8000 \"");
            command2+=tempLeft+"\"  \""+outWavName_left+"\"";
            system(command2.c_str());
            
            //sox:pcm_spec->pcm---right
            // string command3("sox \"");
            // command3+=inWavName+"\"  \""+tempright+"\" remix 2";
			string command3("sox ");
            command3+=inWavName+"  "+tempright+" remix 2";
            CallCodecFunction(command3);
            
            //resample:pcm_spec->pcm---right
            string command4("./resample -to 8000 \"");
            command4+=tempright+"\"  \""+outWavName_right+"\"";
            system(command4.c_str());
            
            //删掉转码中间语音文件
            string command5("rm -f \"");
            command5+=tempLeft+"\" \""+tempright+"\"";
            system(command5.c_str());
        }
        if(decodePara->stereOnMode == STEREO_ON_1 || decodePara->stereOnMode == STEREO_ON_3)
        {
            string temp=outWavName;
            size_t dotpos=temp.rfind(".");
            int replaceLen1=temp.length()-dotpos;
            temp.replace(dotpos,replaceLen1,"_mix.wav");
            
            //sox:pcm_spec->pcm---mix
            // string command1("sox \"");
            // command1+=inWavName+"\"  -c 1 \""+temp+"\"";
			string command1("sox ");
            command1+=inWavName+"  -c 1 "+temp;
            CallCodecFunction(command1);
            
            //resample:pcm_spec->pcm---mix
            string command2("./resample -to 8000 \"");
            command2+=temp+"\"  \""+outWavName+"\"";
            system(command2.c_str());
            
            //删掉转码中间语音文件
            string command3("rm -f \"");
            command3+=temp+"\"";
            system(command3.c_str());
        }
        if(decodePara->stereOnMode == STEREO_ON_4)
        {
            string temp=outWavName;
            size_t dotpos=temp.rfind(".");
            int replaceLen1=temp.length()-dotpos;
            temp.replace(dotpos,replaceLen1,"_temp.wav");
            
            //sox:pcm_spec->pcm---ethier
            // string command1("sox \"");
            // command1+=inWavName+"\"  \""+temp+"\" remix 1";
			string command1("sox ");
            command1+=inWavName+"  "+temp+" remix 1";
            CallCodecFunction(command1);
            
            //resample:pcm_spec->pcm---ethier
            string command2("./resample -to 8000 \"");
            command2+=temp+"\"  \""+outWavName+"\"";
            system(command2.c_str());
            
            //删掉转码中间语音文件
            string command3("rm -f \"");
            command3+=temp+"\"";
            system(command3.c_str());
        }
    }
}

void Audio2Pcm::HandleVoiceType_vox()
{
    //6k_4bit的vox
    if(decodePara->channel == CHANNEL_MONO) 
    {
        //拷贝一份语音，改为以.vox结尾的名称
        string tempWavName = outWavName;
        size_t dotpos      = tempWavName.rfind(".");
        int    replaceLen  = tempWavName.length()-dotpos;
        string renameCommand("cp \"");

        tempWavName.replace(dotpos,replaceLen,".vox");
        renameCommand+=inWavName+"\" \""+tempWavName+"\"";
        system(renameCommand.c_str());
        
        //转为8k_16bit的语音
        //sox:vox_4k_6bit->pcm
        // string transcodeCommand("sox -e oki-adpcm -b 4 -r 6k \"");
        // transcodeCommand+=tempWavName + "\" -b 16 -r 8000 \"" +outWavName+"\" highpass 10";
        // system(transcodeCommand.c_str());
		string transcodeCommand("sox -e oki-adpcm -b 4 -r 6k ");
        transcodeCommand+=tempWavName + " -b 16 -r 8000 " +outWavName+" highpass 10";
        CallCodecFunction(transcodeCommand);
        
        //删除拷贝后重命名的语音
        string deleteCommand("rm -f \"");
        deleteCommand+=tempWavName+"\"";
        system(deleteCommand.c_str());
    }
    else if(decodePara->channel == CHANNEL_STEREO)
    {
        //TODO
    }
}

void Audio2Pcm::HandleVoiceType_alaw()
{
    //带头的alaw
	if(decodePara->channel == CHANNEL_MONO)   
	{
		//sox:alaw->pcm
		// string command("sox -e a-law -b 8 -r 8k \"");
		// command+=inWavName+"\" -b 16 -r 8000 \""+outWavName+"\"";
		string command("sox -e a-law -b 8 -r 8k ");
		command+=inWavName+" -b 16 -r 8000 "+outWavName;
		CallCodecFunction(command);
	}
	else if(decodePara->channel == CHANNEL_STEREO)
	{
		if(decodePara->stereOnMode == STEREO_ON_2 || decodePara->stereOnMode == STEREO_ON_3)
		{
			//sox:alaw->pcm---left
			// string command1("sox \"");
			// command1+=inWavName+"\" -b 16 -r 8000 \""+outWavName_left+"\" remix 1";
			string command1("sox ");
			command1+=inWavName+" -b 16 -r 8000 "+outWavName_left+" remix 1";
			CallCodecFunction(command1);
			
			//sox:alaw->pcm---right
			// string command2("sox \"");
			// command2+=inWavName+"\" -b 16 -r 8000 \""+outWavName_right+"\" remix 2";
			string command2("sox ");
			command2+=inWavName+" -b 16 -r 8000 "+outWavName_right+" remix 2";
			CallCodecFunction(command2);
		}
		if(decodePara->stereOnMode == STEREO_ON_1 || decodePara->stereOnMode == STEREO_ON_3)
		{
			//sox:alaw->pcm---mix
			// string command("sox \"");
			// command+=inWavName+"\" -b 16 -r 8000 -c 1 \""+outWavName+"\"";
			string command("sox ");
			command+=inWavName+" -b 16 -r 8000 -c 1 "+outWavName;
			CallCodecFunction(command);
		}
		if(decodePara->stereOnMode == STEREO_ON_4)
		{
			//sox:alaw->pcm---ethier
			// string command("sox \"");
			// command+=inWavName+"\" -b 16 -r 8000 \""+outWavName+"\" remix 1";
			string command("sox ");
			command+=inWavName+" -b 16 -r 8000 "+outWavName+" remix 1";
			CallCodecFunction(command);
		}
	}
}

void Audio2Pcm::HandleVoiceType_alaw_raw()
{
    //没头的alaw，from李征
    if(decodePara->channel == CHANNEL_MONO)   
    {
        //AudioCode:alaw_raw->pcm
        string command("./AudioCode  -i a -o l -if \"");
        command+=inWavName+"\"  -of \""+outWavName+"\"";
        system(command.c_str());
    }
    else if(decodePara->channel == CHANNEL_STEREO)
    {
        //TODO
    }
}

void Audio2Pcm::HandleVoiceType_acm_or_voc()
{
    //adpcm或者标准的voc
    if(decodePara->channel == CHANNEL_MONO)   
    {
        //sox:acm/voc->pcm
        // string command("sox \"");
        // command+=inWavName+"\" -b 16 -r 8000 \""+outWavName+"\"";
		string command("sox ");
        command+=inWavName+" -b 16 -r 8000 "+outWavName;
        CallCodecFunction(command);
    }
    else if(decodePara->channel == CHANNEL_STEREO)
    {
        if(decodePara->stereOnMode == STEREO_ON_2 || decodePara->stereOnMode == STEREO_ON_3)
        {
            //sox:acm/voc->pcm---left
            // string command1("sox \"");
            // command1+=inWavName+"\" -b 16 -r 8000 \""+outWavName_left+"\" remix 1";
			string command1("sox ");
            command1+=inWavName+" -b 16 -r 8000 "+outWavName_left+" remix 1";
            CallCodecFunction(command1);
            
            //sox:acm/voc->pcm---right
            // string command2("sox \"");
            // command2+=inWavName+"\" -b 16 -r 8000 \""+outWavName_right+"\" remix 2";
			string command2("sox ");
            command2+=inWavName+" -b 16 -r 8000 "+outWavName_right+" remix 2";
            CallCodecFunction(command2);
        }
        if(decodePara->stereOnMode == STEREO_ON_1 || decodePara->stereOnMode == STEREO_ON_3)
        {
            //sox:acm/voc->pcm---mix
            //string command("sox \"");
            //command+=inWavName+"\" -b 16 -r 8000 -c 1 \""+outWavName+"\"";
			string command("sox ");
            command+=inWavName+" -b 16 -r 8000 -c 1 "+outWavName;
            CallCodecFunction(command);
        }
        if(decodePara->stereOnMode == STEREO_ON_4)
        {
            //sox:acm/voc->pcm---ethier
            //string command("sox \"");
            //command+=inWavName+"\" -b 16 -r 8000 \""+outWavName+"\" remix 1";
			string command("sox ");
            command+=inWavName+" -b 16 -r 8000 "+outWavName+" remix 1";
            CallCodecFunction(command);
        }
    }
}

void Audio2Pcm::HandleVoiceType_mp3_or_8k()
{
    //MP3语音
    if(decodePara->channel == CHANNEL_MONO)   
    {
        string tempWavName=outWavName;
        size_t dotpos=tempWavName.rfind(".");
        int replaceLen=tempWavName.length()-dotpos;
        tempWavName.replace(dotpos,replaceLen,".pcm");
        
        //ffmpeg:mp3->pcm
        //string command1("ffmpeg  -i \"");
        //command1+=inWavName+"\" -f wav \""+tempWavName+"\"";
		string command1("ffmpeg  -i ");
        command1+=inWavName+" -f wav "+tempWavName;
        CallCodecFunction(command1);
        
        //sox:pcm->pcm_128
        //string command2("sox \"");
        //command2+=tempWavName+"\" -b 16 -r 8000 \""+outWavName+"\"";
		string command2("sox ");
        command2+=tempWavName+" -b 16 -r 8000 "+outWavName;
        CallCodecFunction(command2);

        //删掉转码中间语音文件
        string command3("rm -f \"");
        command3+=tempWavName+"\"";
        system(command3.c_str());           
    }
    else if(decodePara->channel == CHANNEL_STEREO)                   
    {
        string tempWavName=outWavName;
        size_t dotpos=tempWavName.rfind(".");
        int replaceLen=tempWavName.length()-dotpos;
        tempWavName.replace(dotpos,replaceLen,".pcm");
    
        //ffmpeg:mp3->pcm
        //string command1("ffmpeg  -i \"");
        //command1+=inWavName+"\" -f wav \""+tempWavName+"\"";
		string command1("ffmpeg  -i ");
        command1+=inWavName+" -f wav "+tempWavName;
        CallCodecFunction(command1);
            
        if(decodePara->stereOnMode == STEREO_ON_2 || decodePara->stereOnMode == STEREO_ON_3)
        {
            //sox:pcm->pcm---left
            //string command2("sox \"");
            //command2+=tempWavName+"\" -b 16 -r 8000 \""+outWavName_left+"\" remix 1";
			string command2("sox ");
            command2+=tempWavName+" -b 16 -r 8000 "+outWavName_left+" remix 1";
            CallCodecFunction(command2);
            
            //sox:pcm->pcm---right
            //string command3("sox \"");
            //command3+=tempWavName+"\" -b 16 -r 8000 \""+outWavName_right+"\" remix 2";
			string command3("sox ");
            command3+=tempWavName+" -b 16 -r 8000 "+outWavName_right+" remix 2";
            CallCodecFunction(command3);
        }
        if(decodePara->stereOnMode == STEREO_ON_1 || decodePara->stereOnMode ==STEREO_ON_3)
        {
            //sox:pcm->pcm---mix
            //string command("sox \"");
            //command+=tempWavName+"\" -b 16 -r 8000 -c 1 \""+outWavName+"\"";
			string command("sox ");
            command+=tempWavName+" -b 16 -r 8000 -c 1 "+outWavName;
            CallCodecFunction(command);
        }
        if(decodePara->stereOnMode == STEREO_ON_4)
        {
            //sox:pcm->pcm---ethier
            //string command("sox \"");
            //command+=tempWavName+"\" -b 16 -r 8000 \""+outWavName+"\" remix 1";
			string command("sox ");
            command+=tempWavName+" -b 16 -r 8000 "+outWavName+" remix 1";
            CallCodecFunction(command);
        }
        //删掉转码中间语音文件
        string command4("rm -f \"");
        command4+=tempWavName+"\"";
        system(command4.c_str());
    }
}

void Audio2Pcm::HandleVoiceType_raw()
{
    //没头的128kbps的pcm
    if(decodePara->channel == CHANNEL_MONO)   
    {
        //add_header:raw->pcm
        string command("./add_header \"");
        command+=inWavName+"\"  \""+outWavName+"\" 1 8000 16";
        system(command.c_str());
    }
    else if(decodePara->channel == CHANNEL_STEREO)
    {
        if(decodePara->stereOnMode == STEREO_ON_1)
        {
            //SplitChannel:raw->pcm---mix
            string command("./SplitChannel -f \"");
            command+=inWavName+"\"  -rate 8000  -om  \""+outWavName+"\"";
            system(command.c_str());
        }
        if(decodePara->stereOnMode == STEREO_ON_2)
        {
            //SplitChannel:raw->pcm---left,right
            string command("./SplitChannel -f \"");
            command+=inWavName+"\"  -rate 8000 -ol \""+outWavName_left+"\" -or  \""+outWavName_right+"\"";
            system(command.c_str());
        }
        if(decodePara->stereOnMode == STEREO_ON_3)
        {
            //SplitChannel:raw->pcm---left,right,mix
            string command("./SplitChannel -f \"");
            command+=inWavName+"\"  -rate 8000 -ol \""+outWavName_left+"\" -or  \""+outWavName_right+"\" -om \""+outWavName+"\"";
            system(command.c_str());
        }
        if(decodePara->stereOnMode == STEREO_ON_4)
        {
            //SplitChannel:raw->pcm---ethier
            string command("./SplitChannel -f \"");
            command+=inWavName+"\"  -rate 8000 -ol \""+outWavName+"\"";
            system(command.c_str());
        }
    }
}

void Audio2Pcm::HandleVoiceType_ffmpeg_8kbps()
{
    //其他的可以用ffmpeg统一解码的压缩语音
    if(decodePara->channel == CHANNEL_MONO)   
    {
        string tempWavName=outWavName;
        size_t dotpos=tempWavName.rfind(".");
        int replaceLen=tempWavName.length()-dotpos;
        tempWavName.replace(dotpos,replaceLen,".pcm");
        
        //ffmpeg:8kbps->pcm
        //string command1("ffmpeg  -i \"");
        //command1+=inWavName+"\" -vn -ar 8000 -ac 1 -ab 128 -f wav \""+tempWavName+"\"";
		string command1("ffmpeg  -i ");
        command1+=inWavName+" -vn -ar 8000 -ac 1 -ab 128 -f wav "+tempWavName;
        CallCodecFunction(command1);
        
        //sox:pcm->pcm_44
        //string command2("sox \"");
        //command2+=tempWavName+"\" -b 16 -r 8000 \""+outWavName+"\"";
		string command2("sox ");
        command2+=tempWavName+" -b 16 -r 8000 "+outWavName;
        CallCodecFunction(command2);
        
        //删掉转码中间语音文件
        string command3("rm -f \"");
        command3+=tempWavName+"\"";
        system(command3.c_str());
    }
    else if(decodePara->channel == CHANNEL_STEREO)
    {
        if(decodePara->stereOnMode == STEREO_ON_2 || decodePara->stereOnMode == STEREO_ON_3)
        {
            string tempWavName=outWavName;
            size_t dotpos=tempWavName.rfind(".");
            int replaceLen=tempWavName.length()-dotpos;
            tempWavName.replace(dotpos,replaceLen,"_temp.pcm");
            
            //ffmpeg:8kbps->pcm
            //string command1("ffmpeg  -i \"");
            //command1+=inWavName+"\" -vn -ar 8000 -ac 2 -ab 256 -f wav \""+tempWavName+"\"";
			string command1("ffmpeg  -i ");
            command1+=inWavName+" -vn -ar 8000 -ac 2 -ab 256 -f wav "+tempWavName;
            CallCodecFunction(command1);
        
            //sox:pcm->pcm---left
            //string command2("sox \"");
            //command2+=tempWavName+"\" -b 16 -r 8000 \""+outWavName_left+"\" remix 1";
			string command2("sox ");
            command2+=tempWavName+" -b 16 -r 8000 "+outWavName_left+" remix 1";
            CallCodecFunction(command2);
            
            //sox:pcm->pcm---right
            //string command3("sox \"");
            //command3+=tempWavName+"\" -b 16 -r 8000 \""+outWavName_right+"\" remix 2";
			string command3("sox ");
            command3+=tempWavName+" -b 16 -r 8000 "+outWavName_right+" remix 2";
            CallCodecFunction(command3);
            
            //删掉转码中间语音文件
            string command4("rm -f \"");
            command4+=tempWavName+"\"";
            system(command4.c_str());
        }
        if(decodePara->stereOnMode == STEREO_ON_1 || decodePara->stereOnMode == STEREO_ON_3)
        {
            string tempWavName=outWavName;
            size_t dotpos=tempWavName.rfind(".");
            int replaceLen=tempWavName.length()-dotpos;
            tempWavName.replace(dotpos,replaceLen,"_temp.pcm");
            
            //ffmpeg:8kbps->pcm
            //string command1("ffmpeg  -i \"");
            //command1+=inWavName+"\" -vn -ar 8000 -ac 1 -ab 128 -f wav \""+tempWavName+"\"";
			string command1("ffmpeg  -i ");
            command1+=inWavName+" -vn -ar 8000 -ac 1 -ab 128 -f wav "+tempWavName;
            CallCodecFunction(command1);
                
            //sox:pcm->pcm---mix
            //string command2("sox \"");
            //command2+=tempWavName+"\" -b 16 -r 8000 \""+outWavName+"\"";
			string command2("sox ");
            command2+=tempWavName+" -b 16 -r 8000 "+outWavName;
            CallCodecFunction(command2);
            
            //删掉转码中间语音文件
            string command3("rm -f \"");
            command3+=tempWavName+"\"";
            system(command3.c_str());
            
        }
        if(decodePara->stereOnMode == STEREO_ON_4)
        {
            string tempWavName=outWavName;
            size_t dotpos=tempWavName.rfind(".");
            int replaceLen=tempWavName.length()-dotpos;
            tempWavName.replace(dotpos,replaceLen,"_temp.pcm");
            
            //ffmpeg:8kbps->pcm
            //string command1("ffmpeg  -i \"");
			//command1+=inWavName+"\" -vn -ar 8000 -ac 2 -ab 256 -f wav \""+tempWavName+"\"";
			string command1("ffmpeg  -i ");
            command1+=inWavName+" -vn -ar 8000 -ac 2 -ab 256 -f wav "+tempWavName;
            CallCodecFunction(command1);
        
            //sox:pcm->pcm---ethier
            //string command2("sox \"");
            //command2+=tempWavName+"\" -b 16 -r 8000 \""+outWavName+"\" remix 1";
			string command2("sox ");
            command2+=tempWavName+" -b 16 -r 8000 "+outWavName+" remix 1";
            CallCodecFunction(command2);
            
            //删掉转码中间语音文件
            string command3("rm -f \"");
            command3+=tempWavName+"\"";
            system(command3.c_str());
        }
    }
}

void Audio2Pcm::CallCodecFunction(string &cmd)
{
    std::vector<std::string> vct;
    SplitString(cmd, vct, " ");

    cout << "Command: "<< cmd << endl;
    int   argc = vct.size();
    char  *argv[argc];

    for(int i = 0; i < argc; i++)
    {
        argv[i] = new char[vct[i].length() + 1];
        printf("vct[%d] %s \n", i, vct[i].c_str());
        memcpy(argv[i], vct[i].c_str(), vct[i].length());
        argv[i][vct[i].length()] = 0;
    }

    if ("sox" == vct[0])
    {
        main_sox(vct.size(), argv);
    }
    else if ("ffmpeg" == vct[0])
    {
        cout << "1. ffmpeg --- " << endl;
        main_ffmpeg(vct.size(), argv);
        cout << "2. ffmpeg --- " << endl;
    }
    else
    {
        cout << "Don't support the CMD: " << vct[0] << endl;
    }
	
    cout << "ffmpeg finish--- " << endl;
    for(int i = 0; i < argc; i++)
    {
        //delete argv[i];
        argv[i] = NULL;
    } 
}

//将语音转码为8k_16bit_pcm格式
bool Audio2Pcm::audio2pcm(DecodePara *para)
{
	decodePara=para;
	//计算原始语音扩展名
	inWavName	= decodePara->inWavName;
	size_t pathSymbolPos  = inWavName.rfind("/");
	size_t dotpos         = inWavName.rfind(".");
	string basename       = inWavName.substr(pathSymbolPos+1, dotpos-pathSymbolPos-1);
	string extension      = inWavName.substr(dotpos + 1, inWavName.length() - dotpos - 1);

	//计算转码后语音名
	string outPath = decodePara->outPath;
	if (CHANNEL_MONO == decodePara->channel)
	{
		outWavName = outPath + "/" + basename + ".wav";
	}
	else if (CHANNEL_STEREO == decodePara->channel)
	{
		outWavName       = outPath + "/" + basename + ".wav";
		outWavName_left  = outPath + "/" + basename + "_left.wav";
		outWavName_right = outPath + "/" + basename + "_right.wav";
	}

	//语音转码
	switch(decodePara->trueFormat)
	{
		case pcm:
			HandleVoiceType_pcm();
			break;
		case pcm_spec:
			HandleVoiceType_pcm_spec();
			break;
		case vox:
			HandleVoiceType_vox();
			break;
		case alaw:
			HandleVoiceType_alaw();
			break;
		case alaw_raw:
			HandleVoiceType_alaw_raw();
			break;
		case acm:
		case voc:
			HandleVoiceType_acm_or_voc();
			break;
		case mp3:
		case mp3_8k:
			HandleVoiceType_mp3_or_8k();
			break;
		case raw:
			HandleVoiceType_raw();
			break;
		case ffmpeg_8kbps:
			HandleVoiceType_ffmpeg_8kbps();
			break;
		default:
			break;	
	}
		
	//检查转码是否正确
	if(decodePara->channel == CHANNEL_MONO)   
	{
		if(!access(outWavName.c_str(),0))
		{
			//printf("%s exits \n",outWavName.c_str());
			return true;
		}
		else
		{
			printf("Warning: transcode %s fail......\n",inWavName.c_str());	
			//TOOLS_LOG_WARN("transcode %s fail......\n", inWavName.c_str());	
			return false;
		}
	}
	else if(decodePara->channel ==  CHANNEL_STEREO)
	{
		if(decodePara->stereOnMode == STEREO_ON_1 || decodePara->stereOnMode == STEREO_ON_4)
		{
			if(!access(outWavName.c_str(),0))
			{
				//printf("%s exits \n",outWavName.c_str());
				return true;
			}
			else
			{
				printf("Warning: transcode %s fail......\n",inWavName.c_str());	
				//TOOLS_LOG_WARN("transcode %s fail......\n", inWavName.c_str());		
				return false;
			}	
		}
		else if(decodePara->stereOnMode== STEREO_ON_2 || decodePara->stereOnMode == STEREO_ON_3)
		{
			if((!access(outWavName_left.c_str(),0))&&(!access(outWavName_right.c_str(),0)))
			{
				return true;
			}
			else
			{
				printf("Warning: transcode %s fail......\n",inWavName.c_str());	
				//TOOLS_LOG_WARN("transcode %s fail......\n", inWavName.c_str());	
				return false;
			}
		}
	}
	return true;
}

Audio2Pcm::Audio2Pcm()
{
}

Audio2Pcm::~Audio2Pcm()
{
}


/*************************************************************************
 *  function:
 *  description:
 *  para:
 *  return:
 *************************************************************************/
static void SplitString(const std::string& s, std::vector<std::string>& v, const std::string& c)
{
    string_size pos1, pos2;
    pos2 = s.find(c);
    pos1 = 0;

    while(std::string::npos != pos2)
    {
        if (pos1 < pos2)
        {
            v.push_back(s.substr(pos1, pos2 - pos1));
        }

        pos1 = pos2 + c.size();
        pos2 = s.find(c, pos1);
    }

    if(pos1 != s.length())
    {
        v.push_back(s.substr(pos1));
    }
}
