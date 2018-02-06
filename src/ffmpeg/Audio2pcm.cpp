#include "Audio2pcm.h"

Audio2Pcm::Audio2Pcm(int threadNum) : m_threadNum(threadNum)
{
    InitSessionNum(m_threadNum);

    for (int i = 0; i < m_threadNum; i++)
    {
        m_audioBuf.push_back(new AudioBuf());
    }

    m_audioSocket = new SocketInfo(FFMEPG_SOCKET_PORT);
    InitAudioTypeMap();
}

Audio2Pcm::~Audio2Pcm()
{
    for (int i = 0; i < m_threadNum; i++)
    {
        delete m_audioBuf[i];
    }

    delete m_audioSocket;
    m_audioSocket = NULL;
}

void Audio2Pcm::ReadInWavToAudioBuf(int sessionId)
{
    AudioBuf   *AuBufPtr   = m_audioBuf[sessionId];
    DecodePara *decodePara = AuBufPtr->para;

    //将文件内容读入到输出(临时)buf             - zhangpeng
    FILE *fd = fopen(decodePara->inWavName.c_str(), "r");
    AuBufPtr->buf_out_size = fread(AuBufPtr->buf_out, size_t(10000), 1, fd);
    return;
}

void Audio2Pcm::ParseAudioFormatInfo(int sessionId)
{
    AudioBuf    *AuBufPtr   = m_audioBuf[sessionId];
    DecodePara  *decodePara = AuBufPtr->para;

    AVIOReading(const_cast<char *> (decodePara->inWavName.c_str()), decodePara->sessionId);
    string info = GetFormatInfo(decodePara->sessionId);

    PraseAudioInfo audioInfo(info);
    // todo: 优先按照客户的设置来处理
    decodePara->trueFormat  = audioInfo.GetAudioType();
    decodePara->channel     = audioInfo.GetChannelNum();
    decodePara->stereOnMode = audioInfo.GetStereoOnMode();
}


/*************************************************************
* 设置Sox的输入buf, sox编解码时读取对应的buf
**************************************************************/
void Audio2Pcm::SendBufToSox(AudioBuf *AuBufPtr, DecodePara *decodePara)
{
    StoreTitBuff(decodePara->inWavName, AuBufPtr->buf_in, AuBufPtr->buf_in_size);
}

void Audio2Pcm::HandleVoiceType_pcm(int sessionId)
{
    AudioBuf    *AuBufPtr   = m_audioBuf[sessionId];
    DecodePara  *decodePara = AuBufPtr->para;

	//128kbps的pcm 
	if(decodePara->channel == CHANNEL_MONO)   
	{
		 //pcm单声道语音重命名
		string command("cp \"");
		command+=decodePara->inWavName+"\" \""+AuBufPtr->outWavName+"\"";
		system(command.c_str());
	}
	else if(decodePara->channel == CHANNEL_STEREO)
	{
		if(decodePara->stereOnMode == STEREO_ON_1)       //将分录的双声道语音合并成一个合录的单声道语音
		{
			//SplitChannel:pcm->pcm---mix
			string command("./SplitChannel -f \"");
			command+=decodePara->inWavName+"\"  -rate 8000 -om \""+AuBufPtr->outWavName+"\"";
			system(command.c_str());			
		}
		else if(decodePara->stereOnMode == STEREO_ON_2)   //将分录双声道的左右声道解码成两个单声道
		{
			//SplitChannel:pcm->pcm---left,right
			string command("./SplitChannel -f \"");
			command+=decodePara->inWavName+"\"  -rate 8000 -ol \""+AuBufPtr->outWavName_left+"\" -or  \""+AuBufPtr->outWavName_right+"\"";
			system(command.c_str());
		}
		else if(decodePara->stereOnMode == STEREO_ON_3)   //将分录双声道的左右声道解码成两个单声道同时，还需合并成一个单声道
		{
			//SplitChannel:pcm->pcm---left,right,mix
			string command("./SplitChannel -f \"");
			command+=decodePara->inWavName+"\"  -rate 8000 -ol \""+AuBufPtr->outWavName_left+"\" -or  \""+AuBufPtr->outWavName_right+"\" -om \""+AuBufPtr->outWavName+"\"";
			system(command.c_str());			
		}
		else if(decodePara->stereOnMode == STEREO_ON_4)     //合录双声道，左右声道内容一样，都是多个人的对话，则仅保留一个声道内容
		{
			//SplitChannel:pcm->pcm---ethier
			string command("./SplitChannel -f \"");
			command+=decodePara->inWavName+"\"  -rate 8000 -ol \""+AuBufPtr->outWavName+"\"";
			system(command.c_str());
		}
	}
}

/*************************************************************
* 
**************************************************************/
void Audio2Pcm::HandleVoiceType_pcm_spec(int sessionId)
{
    AudioBuf    *AuBufPtr   = m_audioBuf[sessionId];
    DecodePara  *decodePara = AuBufPtr->para;

    ReadInWavToAudioBuf(sessionId);
    //采样率为非8k的pcm
    if(decodePara->channel == CHANNEL_MONO)   
    {
        //resample:pcm_spec->pcm
        string command("./resample -to 8000 \"");
        command+=decodePara->inWavName+"\"  \""+AuBufPtr->outWavName+"\"";
        system(command.c_str());
    }
    else if(decodePara->channel == CHANNEL_STEREO)
    {   
        if(decodePara->stereOnMode == STEREO_ON_2 || decodePara->stereOnMode == STEREO_ON_3)
        {
            string tempLeft=AuBufPtr->outWavName;
            size_t dotpos1=tempLeft.rfind(".");
            int replaceLen1=tempLeft.length()-dotpos1;
            tempLeft.replace(dotpos1,replaceLen1,"_left_temp.wav");
            
            string tempright=AuBufPtr->outWavName;
            size_t dotpos2=tempright.rfind(".");
            int replaceLen2=tempright.length()-dotpos2;
            tempright.replace(dotpos2,replaceLen2,"_right_temp.wav");
            
            //sox:pcm_spec->pcm---left
            // string command1("sox \"");
            // command1+=inWavName+"\"  \""+tempLeft+"\" remix 1";
			string command1(SOX_MML_HEAD);
            command1+=decodePara->inWavName+"  "+tempLeft+" remix 1";
            CallCodecFunction(sessionId, command1);
            
            //resample:pcm_spec->pcm---left
            string command2("./resample -to 8000 \"");
            command2+=tempLeft+"\"  \""+AuBufPtr->outWavName_left+"\"";
            system(command2.c_str());
            
            //sox:pcm_spec->pcm---right
            // string command3("sox \"");
            // command3+=inWavName+"\"  \""+tempright+"\" remix 2";
			string command3(SOX_MML_HEAD);
            command3+=decodePara->inWavName+"  "+tempright+" remix 2";
            CallCodecFunction(sessionId, command3);
            
            //resample:pcm_spec->pcm---right
            string command4("./resample -to 8000 \"");
            command4+=tempright+"\"  \""+AuBufPtr->outWavName_right+"\"";
            system(command4.c_str());
            
            //删掉转码中间语音文件
            string command5("rm -f \"");
            command5+=tempLeft+"\" \""+tempright+"\"";
            system(command5.c_str());
        }
        if(decodePara->stereOnMode == STEREO_ON_1 || decodePara->stereOnMode == STEREO_ON_3)
        {
            string temp=AuBufPtr->outWavName;
            size_t dotpos=temp.rfind(".");
            int replaceLen1=temp.length()-dotpos;
            temp.replace(dotpos,replaceLen1,"_mix.wav");
            
            //sox:pcm_spec->pcm---mix
            // string command1("sox \"");
            // command1+=inWavName+"\"  -c 1 \""+temp+"\"";
			string command1(SOX_MML_HEAD);
            command1+=decodePara->inWavName+"  -c 1 "+temp;
            CallCodecFunction(sessionId, command1);
            
            //resample:pcm_spec->pcm---mix
            string command2("./resample -to 8000 \"");
            command2+=temp+"\"  \""+AuBufPtr->outWavName+"\"";
            system(command2.c_str());
            
            //删掉转码中间语音文件
            string command3("rm -f \"");
            command3+=temp+"\"";
            system(command3.c_str());
        }
        if(decodePara->stereOnMode == STEREO_ON_4)
        {
            string temp=AuBufPtr->outWavName;
            size_t dotpos=temp.rfind(".");
            int replaceLen1=temp.length()-dotpos;
            temp.replace(dotpos,replaceLen1,"_temp.wav");
            
            //sox:pcm_spec->pcm---ethier
            // string command1("sox \"");
            // command1+=inWavName+"\"  \""+temp+"\" remix 1";
			string command1(SOX_MML_HEAD);
            command1+=decodePara->inWavName+"  "+temp+" remix 1";
            CallCodecFunction(sessionId, command1);
            
            //resample:pcm_spec->pcm---ethier
            string command2("./resample -to 8000 \"");
            command2+=temp+"\"  \""+AuBufPtr->outWavName+"\"";
            system(command2.c_str());
            
            //删掉转码中间语音文件
            string command3("rm -f \"");
            command3+=temp+"\"";
            system(command3.c_str());
        }
    }
}

/*************************************************************
* 
**************************************************************/
void Audio2Pcm::HandleVoiceType_vox(int sessionId)
{
    AudioBuf    *AuBufPtr   = m_audioBuf[sessionId];
    DecodePara  *decodePara = AuBufPtr->para;

    //6k_4bit的vox
    if(decodePara->channel == CHANNEL_MONO) 
    {
        //拷贝一份语音，改为以.vox结尾的名称
        string tempWavName = AuBufPtr->outWavName;
        size_t dotpos      = tempWavName.rfind(".");
        int    replaceLen  = tempWavName.length()-dotpos;
        string renameCommand("cp \"");

        tempWavName.replace(dotpos,replaceLen,".vox");
        renameCommand+=decodePara->inWavName+"\" \""+tempWavName+"\"";
        system(renameCommand.c_str());
        
        //转为8k_16bit的语音
        //sox:vox_4k_6bit->pcm
        // string transcodeCommand("sox -e oki-adpcm -b 4 -r 6k \"");
        // transcodeCommand+=tempWavName + "\" -b 16 -r 8000 \"" +outWavName+"\" highpass 10";
        // system(transcodeCommand.c_str());
		string transcodeCommand(SOX_MML_HEAD " -e oki-adpcm -b 4 -r 6k ");
        transcodeCommand+=tempWavName + " -b 16 -r 8000 " +AuBufPtr->outWavName+" highpass 10";
        CallCodecFunction(sessionId, transcodeCommand);
        
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

/*************************************************************
* 
**************************************************************/
void Audio2Pcm::HandleVoiceType_alaw(int sessionId)
{
    AudioBuf    *AuBufPtr   = m_audioBuf[sessionId];
    DecodePara  *decodePara = AuBufPtr->para;

    ReadInWavToAudioBuf(sessionId);

    //带头的alaw
	if(decodePara->channel == CHANNEL_MONO)   
	{
		//sox:alaw->pcm
		// string command("sox -e a-law -b 8 -r 8k \"");
		// command+=inWavName+"\" -b 16 -r 8000 \""+outWavName+"\"";
		string command(SOX_MML_HEAD " -e a-law -b 8 -r 8k ");
		command+=decodePara->inWavName+" -b 16 -r 8000 "+AuBufPtr->outWavName;
		CallCodecFunction(sessionId, command);
	}
	else if(decodePara->channel == CHANNEL_STEREO)
	{
		if(decodePara->stereOnMode == STEREO_ON_2 || decodePara->stereOnMode == STEREO_ON_3)
		{
			//sox:alaw->pcm---left
			// string command1("sox \"");
			// command1+=inWavName+"\" -b 16 -r 8000 \""+outWavName_left+"\" remix 1";
			string command1(SOX_MML_HEAD);
			command1+=decodePara->inWavName+" -b 16 -r 8000 "+AuBufPtr->outWavName_left+" remix 1";
			CallCodecFunction(sessionId, command1);
			
			//sox:alaw->pcm---right
			// string command2("sox \"");
			// command2+=inWavName+"\" -b 16 -r 8000 \""+outWavName_right+"\" remix 2";
			string command2(SOX_MML_HEAD);
			command2+=decodePara->inWavName+" -b 16 -r 8000 "+AuBufPtr->outWavName_right+" remix 2";
			CallCodecFunction(sessionId, command2);
		}
		if(decodePara->stereOnMode == STEREO_ON_1 || decodePara->stereOnMode == STEREO_ON_3)
		{
			//sox:alaw->pcm---mix
			// string command("sox \"");
			// command+=inWavName+"\" -b 16 -r 8000 -c 1 \""+outWavName+"\"";
			string command(SOX_MML_HEAD);
			command+=decodePara->inWavName+" -b 16 -r 8000 -c 1 "+AuBufPtr->outWavName;
			CallCodecFunction(sessionId, command);
		}
		if(decodePara->stereOnMode == STEREO_ON_4)
		{
			//sox:alaw->pcm---ethier
			// string command("sox \"");
			// command+=inWavName+"\" -b 16 -r 8000 \""+outWavName+"\" remix 1";
			string command(SOX_MML_HEAD);
			command+=decodePara->inWavName+" -b 16 -r 8000 "+AuBufPtr->outWavName+" remix 1";
			CallCodecFunction(sessionId, command);
		}
	}
}

void Audio2Pcm::HandleVoiceType_alaw_raw(int sessionId)
{
    AudioBuf    *AuBufPtr   = m_audioBuf[sessionId];
    DecodePara  *decodePara = AuBufPtr->para;

    //没头的alaw，from李征
    if(decodePara->channel == CHANNEL_MONO)   
    {
        //AudioCode:alaw_raw->pcm
        string command("./AudioCode  -i a -o l -if \"");
        command+=decodePara->inWavName+"\"  -of \""+AuBufPtr->outWavName+"\"";
        system(command.c_str());
    }
    else if(decodePara->channel == CHANNEL_STEREO)
    {
        //TODO
    }
}

/*************************************************************
* 
**************************************************************/
void Audio2Pcm::HandleVoiceType_acm_or_voc(int sessionId)
{
    AudioBuf    *AuBufPtr   = m_audioBuf[sessionId];
    DecodePara  *decodePara = AuBufPtr->para;

    ReadInWavToAudioBuf(sessionId);

    //adpcm或者标准的voc
    if(decodePara->channel == CHANNEL_MONO)   
    {
        //sox:acm/voc->pcm
        // string command("sox \"");
        // command+=inWavName+"\" -b 16 -r 8000 \""+outWavName+"\"";
		string command(SOX_MML_HEAD);
        command+=decodePara->inWavName+" -b 16 -r 8000 "+AuBufPtr->outWavName;
        CallCodecFunction(sessionId, command);
    }
    else if(decodePara->channel == CHANNEL_STEREO)
    {
        if(decodePara->stereOnMode == STEREO_ON_2 || decodePara->stereOnMode == STEREO_ON_3)
        {
            //sox:acm/voc->pcm---left
            // string command1("sox \"");
            // command1+=inWavName+"\" -b 16 -r 8000 \""+outWavName_left+"\" remix 1";
			string command1(SOX_MML_HEAD);
            command1+=decodePara->inWavName+" -b 16 -r 8000 "+AuBufPtr->outWavName_left+" remix 1";
            CallCodecFunction(sessionId, command1);
            
            //sox:acm/voc->pcm---right
            // string command2("sox \"");
            // command2+=inWavName+"\" -b 16 -r 8000 \""+outWavName_right+"\" remix 2";
			string command2(SOX_MML_HEAD);
            command2+=decodePara->inWavName+" -b 16 -r 8000 "+AuBufPtr->outWavName_right+" remix 2";
            CallCodecFunction(sessionId, command2);
        }
        if(decodePara->stereOnMode == STEREO_ON_1 || decodePara->stereOnMode == STEREO_ON_3)
        {
            //sox:acm/voc->pcm---mix
            //string command("sox \"");
            //command+=inWavName+"\" -b 16 -r 8000 -c 1 \""+outWavName+"\"";
			string command(SOX_MML_HEAD);
            command+=decodePara->inWavName+" -b 16 -r 8000 -c 1 "+AuBufPtr->outWavName;
            CallCodecFunction(sessionId, command);
        }
        if(decodePara->stereOnMode == STEREO_ON_4)
        {
            //sox:acm/voc->pcm---ethier
            //string command("sox \"");
            //command+=inWavName+"\" -b 16 -r 8000 \""+outWavName+"\" remix 1";
			string command(SOX_MML_HEAD);
            command+=decodePara->inWavName+" -b 16 -r 8000 "+AuBufPtr->outWavName+" remix 1";
            CallCodecFunction(sessionId, command);
        }
    }
}

/*************************************************************
* 
**************************************************************/
void Audio2Pcm::HandleVoiceType_mp3_or_8k(int sessionId)
{
    AudioBuf    *AuBufPtr   = m_audioBuf[sessionId];
    DecodePara  *decodePara = AuBufPtr->para;

    //MP3语音
    if(decodePara->channel == CHANNEL_MONO)   
    {
        string tempWavName=AuBufPtr->outWavName;
        size_t dotpos=tempWavName.rfind(".");
        int replaceLen=tempWavName.length()-dotpos;
        tempWavName.replace(dotpos,replaceLen,".pcm");
        
        //ffmpeg:mp3->pcm
        //string command1("ffmpeg  -i \"");
        //command1+=inWavName+"\" -f wav \""+tempWavName+"\"";
		string command1(FFMEPG_MML_HEAD " -i ");
        command1+=decodePara->inWavName+" -f wav "+tempWavName;
        CallCodecFunction(sessionId, command1);
        
        //sox:pcm->pcm_128
        //string command2("sox \"");
        //command2+=tempWavName+"\" -b 16 -r 8000 \""+outWavName+"\"";
		string command2(SOX_MML_HEAD);
        command2+=tempWavName+" -b 16 -r 8000 "+AuBufPtr->outWavName;
        CallCodecFunction(sessionId, command2);

        //删掉转码中间语音文件
        string command3("rm -f \"");
        command3+=tempWavName+"\"";
        system(command3.c_str());           
    }
    else if(decodePara->channel == CHANNEL_STEREO)                   
    {
        string tempWavName=AuBufPtr->outWavName;
        size_t dotpos=tempWavName.rfind(".");
        int replaceLen=tempWavName.length()-dotpos;
        tempWavName.replace(dotpos,replaceLen,".pcm");
    
        //ffmpeg:mp3->pcm
        //string command1("ffmpeg  -i \"");
        //command1+=inWavName+"\" -f wav \""+tempWavName+"\"";
		string command1(FFMEPG_MML_HEAD " -i ");
        command1+=decodePara->inWavName+" -f wav "+tempWavName;
        CallCodecFunction(sessionId, command1);
            
        if(decodePara->stereOnMode == STEREO_ON_2 || decodePara->stereOnMode == STEREO_ON_3)
        {
            //sox:pcm->pcm---left
            //string command2("sox \"");
            //command2+=tempWavName+"\" -b 16 -r 8000 \""+outWavName_left+"\" remix 1";
			string command2(SOX_MML_HEAD);
            command2+=tempWavName+" -b 16 -r 8000 "+AuBufPtr->outWavName_left+" remix 1";
            CallCodecFunction(sessionId, command2);
            
            //sox:pcm->pcm---right
            //string command3("sox \"");
            //command3+=tempWavName+"\" -b 16 -r 8000 \""+outWavName_right+"\" remix 2";
			string command3(SOX_MML_HEAD);
            command3+=tempWavName+" -b 16 -r 8000 "+AuBufPtr->outWavName_right+" remix 2";
            CallCodecFunction(sessionId, command3);
        }
        if(decodePara->stereOnMode == STEREO_ON_1 || decodePara->stereOnMode ==STEREO_ON_3)
        {
            //sox:pcm->pcm---mix
            //string command("sox \"");
            //command+=tempWavName+"\" -b 16 -r 8000 -c 1 \""+outWavName+"\"";
			string command(SOX_MML_HEAD);
            command+=tempWavName+" -b 16 -r 8000 -c 1 "+AuBufPtr->outWavName;
            CallCodecFunction(sessionId, command);
        }
        if(decodePara->stereOnMode == STEREO_ON_4)
        {
            //sox:pcm->pcm---ethier
            //string command("sox \"");
            //command+=tempWavName+"\" -b 16 -r 8000 \""+outWavName+"\" remix 1";
			string command(SOX_MML_HEAD);
            command+=tempWavName+" -b 16 -r 8000 "+AuBufPtr->outWavName+" remix 1";
            CallCodecFunction(sessionId, command);
        }
        //删掉转码中间语音文件
        string command4("rm -f \"");
        command4+=tempWavName+"\"";
        system(command4.c_str());
    }
}

/*************************************************************
* 
**************************************************************/
void Audio2Pcm::HandleVoiceType_raw(int sessionId)
{
    AudioBuf    *AuBufPtr   = m_audioBuf[sessionId];
    DecodePara  *decodePara = AuBufPtr->para;

    //没头的128kbps的pcm
    if(decodePara->channel == CHANNEL_MONO)   
    {
        //add_header:raw->pcm
        string command("./add_header \"");
        command+=decodePara->inWavName+"\"  \""+AuBufPtr->outWavName+"\" 1 8000 16";
        system(command.c_str());
    }
    else if(decodePara->channel == CHANNEL_STEREO)
    {
        if(decodePara->stereOnMode == STEREO_ON_1)
        {
            //SplitChannel:raw->pcm---mix
            string command("./SplitChannel -f \"");
            command+=decodePara->inWavName+"\"  -rate 8000  -om  \""+AuBufPtr->outWavName+"\"";
            system(command.c_str());
        }
        if(decodePara->stereOnMode == STEREO_ON_2)
        {
            //SplitChannel:raw->pcm---left,right
            string command("./SplitChannel -f \"");
            command+=decodePara->inWavName+"\"  -rate 8000 -ol \""+AuBufPtr->outWavName_left+"\" -or  \""+AuBufPtr->outWavName_right+"\"";
            system(command.c_str());
        }
        if(decodePara->stereOnMode == STEREO_ON_3)
        {
            //SplitChannel:raw->pcm---left,right,mix
            string command("./SplitChannel -f \"");
            command+=decodePara->inWavName+"\"  -rate 8000 -ol \""+AuBufPtr->outWavName_left+"\" -or  \""+AuBufPtr->outWavName_right+"\" -om \""+AuBufPtr->outWavName+"\"";
            system(command.c_str());
        }
        if(decodePara->stereOnMode == STEREO_ON_4)
        {
            //SplitChannel:raw->pcm---ethier
            string command("./SplitChannel -f \"");
            command+=decodePara->inWavName+"\"  -rate 8000 -ol \""+AuBufPtr->outWavName+"\"";
            system(command.c_str());
        }
    }
}

/*************************************************************
* 
**************************************************************/
void Audio2Pcm::HandleVoiceType_ffmpeg_8kbps(int sessionId)
{
    AudioBuf    *AuBufPtr   = m_audioBuf[sessionId];
    DecodePara  *decodePara = AuBufPtr->para;

    //其他的可以用ffmpeg统一解码的压缩语音
    if(decodePara->channel == CHANNEL_MONO)   
    {
        string tempWavName=AuBufPtr->outWavName;
        size_t dotpos=tempWavName.rfind(".");
        int replaceLen=tempWavName.length()-dotpos;
        tempWavName.replace(dotpos,replaceLen,".pcm");
        
        //ffmpeg:8kbps->pcm
        //string command1("ffmpeg  -i \"");
        //command1+=inWavName+"\" -vn -ar 8000 -ac 1 -ab 128 -f wav \""+tempWavName+"\"";
		string command1(FFMEPG_MML_HEAD " -i ");
        command1+=decodePara->inWavName+" -vn -ar 8000 -ac 1 -ab 128 -f wav "+tempWavName;
        CallCodecFunction(sessionId, command1);
        
        //sox:pcm->pcm_44
        //string command2("sox \"");
        //command2+=tempWavName+"\" -b 16 -r 8000 \""+outWavName+"\"";
		string command2(SOX_MML_HEAD);
        command2+=tempWavName+" -b 16 -r 8000 "+AuBufPtr->outWavName;
        CallCodecFunction(sessionId, command2);
        
        //删掉转码中间语音文件
        string command3("rm -f \"");
        command3+=tempWavName+"\"";
        system(command3.c_str());
    }
    else if(decodePara->channel == CHANNEL_STEREO)
    {
        if(decodePara->stereOnMode == STEREO_ON_2 || decodePara->stereOnMode == STEREO_ON_3)
        {
            string tempWavName=AuBufPtr->outWavName;
            size_t dotpos=tempWavName.rfind(".");
            int replaceLen=tempWavName.length()-dotpos;
            tempWavName.replace(dotpos,replaceLen,"_temp.pcm");
            
            //ffmpeg:8kbps->pcm
            //string command1("ffmpeg  -i \"");
            //command1+=inWavName+"\" -vn -ar 8000 -ac 2 -ab 256 -f wav \""+tempWavName+"\"";
			string command1(FFMEPG_MML_HEAD " -i ");
            command1+=decodePara->inWavName+" -vn -ar 8000 -ac 2 -ab 256 -f wav "+tempWavName;
            CallCodecFunction(sessionId, command1);
            
            //sox:pcm->pcm---left
            //string command2("sox \"");
            //command2+=tempWavName+"\" -b 16 -r 8000 \""+outWavName_left+"\" remix 1";
			string command2(SOX_MML_HEAD);
            command2+=tempWavName+" -b 16 -r 8000 "+AuBufPtr->outWavName_left+" remix 1";
            CallCodecFunction(sessionId, command2);
            
            //sox:pcm->pcm---right
            //string command3("sox \"");
            //command3+=tempWavName+"\" -b 16 -r 8000 \""+outWavName_right+"\" remix 2";
			string command3(SOX_MML_HEAD);
            command3+=tempWavName+" -b 16 -r 8000 "+AuBufPtr->outWavName_right+" remix 2";
            CallCodecFunction(sessionId, command3);
            
            //删掉转码中间语音文件
            string command4("rm -f \"");
            command4+=tempWavName+"\"";
            system(command4.c_str());
        }
        if(decodePara->stereOnMode == STEREO_ON_1 || decodePara->stereOnMode == STEREO_ON_3)
        {
            string tempWavName=AuBufPtr->outWavName;
            size_t dotpos=tempWavName.rfind(".");
            int replaceLen=tempWavName.length()-dotpos;
            tempWavName.replace(dotpos,replaceLen,"_temp.pcm");
            
            //ffmpeg:8kbps->pcm
            //string command1("ffmpeg  -i \"");
            //command1+=inWavName+"\" -vn -ar 8000 -ac 1 -ab 128 -f wav \""+tempWavName+"\"";
			string command1(FFMEPG_MML_HEAD " -i ");
            command1+=decodePara->inWavName+" -vn -ar 8000 -ac 1 -ab 128 -f wav "+tempWavName;
            CallCodecFunction(sessionId, command1);
                
            //sox:pcm->pcm---mix
            //string command2("sox \"");
            //command2+=tempWavName+"\" -b 16 -r 8000 \""+outWavName+"\"";
			string command2(SOX_MML_HEAD);
            command2+=tempWavName+" -b 16 -r 8000 "+AuBufPtr->outWavName;
            CallCodecFunction(sessionId, command2);
            
            //删掉转码中间语音文件
            string command3("rm -f \"");
            command3+=tempWavName+"\"";
            system(command3.c_str());
            
        }
        if(decodePara->stereOnMode == STEREO_ON_4)
        {
            string tempWavName=AuBufPtr->outWavName;
            size_t dotpos=tempWavName.rfind(".");
            int replaceLen=tempWavName.length()-dotpos;
            tempWavName.replace(dotpos,replaceLen,"_temp.pcm");
            
            //ffmpeg:8kbps->pcm
            //string command1("ffmpeg  -i \"");
			//command1+=inWavName+"\" -vn -ar 8000 -ac 2 -ab 256 -f wav \""+tempWavName+"\"";
			string command1(FFMEPG_MML_HEAD " -i ");
            command1+=decodePara->inWavName+" -vn -ar 8000 -ac 2 -ab 256 -f wav "+tempWavName;
            CallCodecFunction(sessionId, command1);
        
            //sox:pcm->pcm---ethier
            //string command2("sox \"");
            //command2+=tempWavName+"\" -b 16 -r 8000 \""+outWavName+"\" remix 1";
			string command2(SOX_MML_HEAD);
            command2+=tempWavName+" -b 16 -r 8000 "+AuBufPtr->outWavName+" remix 1";
            CallCodecFunction(sessionId, command2);
            
            //删掉转码中间语音文件
            string command3("rm -f \"");
            command3+=tempWavName+"\"";
            system(command3.c_str());
        }
    }
}

/*************************************************************
* PRIVATE 将上一次的输入buf释放，并将输出当作本次的输入
**************************************************************/
void Audio2Pcm::ExchangeBufOut2In(int sessionId)
{
    AudioBuf *AuBufPtr = m_audioBuf[sessionId];

    delete AuBufPtr->buf_in;
    AuBufPtr->buf_in      = AuBufPtr->buf_out;
    AuBufPtr->buf_in_size = AuBufPtr->buf_out_size;

    AuBufPtr->buf_out = NULL;
    AuBufPtr->buf_out_size = 0;
    return;
}

/*************************************************************
* PRIVATE 调用编码器对输入的语音进行转码，并获取输出的结果
**************************************************************/
void Audio2Pcm::CallCodecFunction(int sessionId, string &cmd)
{
    AudioBuf    *AuBufPtr   = m_audioBuf[sessionId];
    DecodePara  *decodePara = AuBufPtr->para;

    std::vector<std::string> vct;
    SplitString(cmd, vct, " ");

    int   argc = vct.size();
    char  *argv[argc];

    for(int i = 0; i < argc; i++)
    {
        argv[i] = new char[vct[i].length() + 1];
        memcpy(argv[i], vct[i].c_str(), vct[i].length());
        argv[i][vct[i].length()] = 0;
    }

    if ("sox" == vct[0])
    {
        //将上次的输出作为本次的输入
        ExchangeBufOut2In(sessionId);
        SendBufToSox(AuBufPtr, decodePara);

        //开始使用 sox 对语音进行编解码
        SOX_main(vct.size(), argv);

        std::string filename = "";
        // 从命令行中查找输出的文件名 
        for (size_t i = vct.size(); i > 0; i--)
        {
            if (vct[i-1].find_first_of(".") != std::string::npos)
            {
                filename = vct[i-1];
                break;
            }
        }
        //获取编解码器输出的语音
        AuBufPtr->buf_out_size = GetSoxBufLen(filename);
        AuBufPtr->buf_out      = new char[AuBufPtr->buf_out_size];
        CopySoxBuf(filename, AuBufPtr->buf_out, AuBufPtr->buf_out_size);
    }
    else if ("ffmpeg" == vct[0])
    {
        string sid = int2str(sessionId);

        //在对应的线程队列插入一条任务
        m_audioSocket->InsertOneTask(sid, cmd);

        //获取编解码器的输出
        AuBufPtr->buf_out_size = m_audioSocket->GetFFmpegBuffLen(sid);
        AuBufPtr->buf_out      = new char[AuBufPtr->buf_out_size];
        m_audioSocket->GetFFmpegBuff(sid, AuBufPtr->buf_out, AuBufPtr->buf_out_size);
    }
    else
    {
        LOG_PRINT_WARN("Don't support the CMD: %s", cmd.c_str());
    }
    
    for(int i = 0; i < argc; i++)
    {
        delete[] argv[i];
        argv[i] = NULL;
    }
}

/*************************************************************
* PUBLIC 获取编解码器输出的语音
**************************************************************/
size_t Audio2Pcm::GetAudioBuf(int sessionId, char ** buf_ptr)
{
    AudioBuf *AuBufPtr = m_audioBuf[sessionId];

    *buf_ptr = AuBufPtr->buf_out;
    return AuBufPtr->buf_out_size;
}

/*************************************************************
* PUBLIC 将语音转码为8k_16bit_pcm格式，并输出到buf中
**************************************************************/
bool Audio2Pcm::audio2pcm(DecodePara *para)
{
    if (para->sessionId > m_threadNum)
    {
        return false;
    }

    AudioBuf   *AuBufPtr   = m_audioBuf[para->sessionId];
    DecodePara *decodePara = para;

    AuBufPtr->Cleanup();
    AuBufPtr->para = para;
    ParseAudioFormatInfo(decodePara->sessionId);

    //计算原始语音扩展名
    size_t pathSymbolPos  = decodePara->inWavName.rfind("/");
	size_t dotpos         = decodePara->inWavName.rfind(".");
	string basename       = decodePara->inWavName.substr(pathSymbolPos+1, dotpos-pathSymbolPos-1);
	string extension      = decodePara->inWavName.substr(dotpos + 1, decodePara->inWavName.length() - dotpos - 1);

	//计算转码后语音名
	string outPath = decodePara->outPath;
	if (CHANNEL_MONO == decodePara->channel)
	{
		AuBufPtr->outWavName = outPath + "/" + basename + ".wav";
	}
	else if (CHANNEL_STEREO == decodePara->channel)
	{
		AuBufPtr->outWavName       = outPath + "/" + basename + ".wav";
		AuBufPtr->outWavName_left  = outPath + "/" + basename + "_left.wav";
		AuBufPtr->outWavName_right = outPath + "/" + basename + "_right.wav";
	}

	//语音转码
	switch(decodePara->trueFormat)
	{
		case pcm:
			HandleVoiceType_pcm(decodePara->sessionId);
			break;
		case pcm_spec:
			HandleVoiceType_pcm_spec(decodePara->sessionId);
			break;
		case vox:
			HandleVoiceType_vox(decodePara->sessionId);
			break;
		case alaw:
			HandleVoiceType_alaw(decodePara->sessionId);
			break;
		case alaw_raw:
			HandleVoiceType_alaw_raw(decodePara->sessionId);
			break;
		case acm:
		case voc:
			HandleVoiceType_acm_or_voc(decodePara->sessionId);
			break;
		case mp3:
		case mp3_8k:
			HandleVoiceType_mp3_or_8k(decodePara->sessionId);
			break;
		case raw:
			HandleVoiceType_raw(decodePara->sessionId);
			break;
		case ffmpeg_8kbps:
			HandleVoiceType_ffmpeg_8kbps(decodePara->sessionId);
			break;
		default:
			break;	
	}
		
	//检查转码是否正确
	if(decodePara->channel == CHANNEL_MONO)
	{
		if(!access(AuBufPtr->outWavName.c_str(),0))
		{
			//LOG_PRINT_WARN("%s exits ",outWavName.c_str());
			return true;
		}
		else
		{
			LOG_PRINT_WARN("Warning: transcode %s fail......", decodePara->inWavName.c_str());	
			//TOOLS_LOG_WARN("transcode %s fail......", inWavName.c_str());	
			return false;
		}
	}
	else if(decodePara->channel ==  CHANNEL_STEREO)
	{
		if(decodePara->stereOnMode == STEREO_ON_1 || decodePara->stereOnMode == STEREO_ON_4)
		{
			if(!access(AuBufPtr->outWavName.c_str(),0))
			{
				//LOG_PRINT_WARN("%s exits ",outWavName.c_str());
				return true;
			}
			else
			{
				printf("Warning: transcode %s fail......\n", decodePara->inWavName.c_str());	
				//TOOLS_LOG_WARN("transcode %s fail......\n", inWavName.c_str());		
				return false;
			}	
		}
		else if(decodePara->stereOnMode== STEREO_ON_2 || decodePara->stereOnMode == STEREO_ON_3)
		{
			if( (!access(AuBufPtr->outWavName_left.c_str(),  0)) && 
				(!access(AuBufPtr->outWavName_right.c_str(), 0)))
			{
				return true;
			}
			else
			{
				printf("Warning: transcode %s fail......\n", decodePara->inWavName.c_str());	
				//TOOLS_LOG_WARN("transcode %s fail......\n", inWavName.c_str());	
				return false;
			}
		}
	}
	return true;
}

